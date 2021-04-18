#include "scheduler.h"
#include "coroutine.h"
#include "Util.h"
#include <cassert>

using namespace burger;

static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Coroutine* t_scheduler_co = nullptr;

// threads: 线程数量
// useCaller: 是否使用当前调用线程,纳入调度中来
Scheduler::Scheduler(size_t threadNum, bool useCaller, const std::string& name)
    : name_(name) {
    assert(threadNum > 0);
    if(useCaller) {
        Coroutine::GetThis();   // 如果当前线程无协程，那么会初始化一个调度协程
        --threadNum;   // todo : why -- here
        assert(Scheduler::GetThis() == nullptr);  // 确定当前线程无线程调度器
        t_scheduler = this;
        rootCo_ = std::move(std::make_shared<Coroutine>(std::bind(&Scheduler::run, this), 0, true));

        t_scheduler_co = rootCo_.get();
        rootThreadId_ = util::tid();
        // threadIdList_.push_back(rootThreadId_);
    } else {
        rootThreadId_ = -1;
    }
    threadCount_ = threadNum;
    INFO("Scheduler::Scheduler created");
}

Scheduler::~Scheduler() {
    assert(stopping_);
    if(GetThis() == this) {
        t_scheduler = nullptr;
    }
}

// 返回当前协程调度器
Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}

// 返回当前协程调度器的调度协程
Coroutine* Scheduler::GetSchedCo() {
    return t_scheduler_co;
}

void Scheduler::start() {
    TRACE("starting {} threads", threadCount_);
    std::lock_guard<std::mutex> lock(mutex_);
    // TODO: There may be a race condition here if one thread calls stop(),
    // and another thread calls start() before the worker threads for this
    // scheduler actually exit; they may resurrect themselves, and the stopping
    // thread would block waiting for the thread to exit

    stopping_ = false;
    assert(threadVec_.empty());
    threadVec_.resize(threadCount_);
    for(size_t i = 0; i < threadCount_; i++) {
        threadVec_[i] = std::move(std::thread(std::bind(&Scheduler::run, this)));
        // threadIdList_.push_back(id); // todo
    }
}

void Scheduler::stop() {
   autoStop_ = true;
    if(rootCo_ && threadCount_ == 0
        && (rootCo_->getState() == Coroutine::State::TERM
        || rootCo_->getState() == Coroutine::State::INIT)) {
        INFO("{} stopped", fmt::ptr(this));
        stopping_ = true;

        if(stopping()) {
            return;
        }
    }

    //bool exit_on_this_co_ = false;
    if(rootThreadId_ != -1) {
        (GetThis() == this);
    } else {
        assert(GetThis() != this);
    }

    stopping_ = true;
    for(size_t i = 0; i < threadCount_; ++i) {
        tickle();
    }

    if(rootCo_) {
        tickle();
    }

    if(rootCo_) {
        if(!stopping()) {
            rootCo_->call();
        }
    }

    std::vector<std::thread> threadList;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        threadList.swap(threadVec_);
    }
    for(auto& thrd : threadList) {
        thrd.join();
    }
}

void Scheduler::switchTo(pid_t threadId) {
    BURGER_ASSERT(Scheduler::GetThis() != nullptr);
    if(Scheduler::GetThis() == this) {
        if(threadId == -1 || threadId == util::tid()) {
            return;
        }
    }
    schedule(Coroutine::GetThis(), threadId);
    Coroutine::YieldToHold();
}

std::ostream& Scheduler::dump(std::ostream& os) {
    os << "[Scheduler name=" << name_
       << " size=" << threadCount_
       << " active_count=" << activeThreadCount_
       << " idle_count=" << idleThreadCount_
       << " stopping=" << stopping_
       << " ]" << std::endl << "    ";
    // for(size_t i = 0; i < threadIdList_.size(); ++i) {
    //     if(i) {
    //         os << ", ";
    //     }
    //     os << threadIdList_[i];
    // }
    return os;
    
}

// 通知协程调度器有任务了
void Scheduler::tickle() {
    INFO("Tickle");
}

bool Scheduler::stopping() {
    std::lock_guard<std::mutex> lock(mutex_);
    return autoStop_ && stopping_ && coList_.empty() && activeThreadCount_ == 0;
}

void Scheduler::idle() {
    INFO("idle");
    while(!stopping()) {
        Coroutine::YieldToHold();
    }
}

void Scheduler::setThis() {
    t_scheduler = this;
}


void Scheduler::run() {
    DEBUG("{} RUN...", name_);
    // set_hook_enable(true);
    setThis();   // 设置当前调度器为协程调度器
    if(util::tid() != rootThreadId_) {
        // todo
        t_scheduler_co = Coroutine::GetThis().get();
    } else {
        BURGER_ASSERT(t_scheduler_co == Coroutine::GetThis().get());
    }
    Coroutine::ptr idleCo = std::make_shared<Coroutine>(std::bind(&Scheduler::idle, this));
    INFO("{} starting thread with idle coroutine", fmt::ptr(this));    
    Coroutine::ptr cbCo;

    coThread coThrd;
    while(true) {
        coThrd.reset();
        bool tickleMe = false;
        bool isActive = false;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = coList_.begin();
            while(it != coList_.end()) {
                if(it->threadId_ != -1 && it->threadId_ != util::tid()) {
                    ++it;
                    tickleMe = true;
                    continue;
                }

                BURGER_ASSERT(it->co_ || it->cb_);
                if(it->co_ && it->co_->getState() == Coroutine::State::EXEC) {
                    ++it;
                    continue;
                }

                coThrd = *it;
                coList_.erase(it++);
                ++activeThreadCount_;
                isActive = true;
                break;
            }
            tickleMe |= it != coList_.end();
        }

        if(tickleMe) {
            tickle();
        }

        if(coThrd.co_ && (coThrd.co_->getState() != Coroutine::State::TERM
                        && coThrd.co_->getState() != Coroutine::State::EXCEPT)) {
            coThrd.co_->swapIn();
            --activeThreadCount_;

            if(coThrd.co_->getState() == Coroutine::State::READY) {
                schedule(coThrd.co_);
            } else if(coThrd.co_->getState() != Coroutine::State::TERM
                    && coThrd.co_->getState() != Coroutine::State::EXCEPT) {
                coThrd.co_->state_ = Coroutine::State::HOLD;
            }
            coThrd.reset();
        } else if(coThrd.cb_) {
            if(cbCo) {
                cbCo->reset(coThrd.cb_);
            } else {
                cbCo = std::move(std::make_shared<Coroutine>(coThrd.cb_));
            }
            coThrd.reset();
            cbCo->swapIn();
            --activeThreadCount_;
            if(cbCo->getState() == Coroutine::State::READY) {
                schedule(cbCo);
                cbCo.reset();
            } else if(cbCo->getState() == Coroutine::State::EXCEPT
                    || cbCo->getState() == Coroutine::State::TERM) {
                cbCo->reset(nullptr);
            } else {//if(coThrd->getState() != Coroutine::State::TERM) {
                cbCo->state_ = Coroutine::State::HOLD;
                cbCo.reset();
            }
        } else {
            if(isActive) {
                --activeThreadCount_;
                continue;
            }
            if(idleCo->getState() == Coroutine::State::TERM) {
                INFO("Idle coroutine terminate");
                break;
            }

            ++idleThreadCount_;
            idleCo->swapIn();
            --idleThreadCount_;
            if(idleCo->getState() != Coroutine::State::TERM
                    && idleCo->getState() != Coroutine::State::EXCEPT) {
                idleCo->state_ = Coroutine::State::HOLD;
            }
        }
    }
}


// coThread ctor
Scheduler::coThread::coThread(const Coroutine::ptr& co, pid_t threadId)
    :co_(co), threadId_(threadId) {
}

Scheduler::coThread::coThread(Coroutine::ptr&& co, pid_t threadId) 
    :co_(std::move(co)), threadId_(threadId) {
}

Scheduler::coThread::coThread(const Callback& cb, pid_t threadId) 
    :cb_(cb), threadId_(threadId) {
}

Scheduler::coThread::coThread(Callback&& cb, pid_t threadId) 
    :cb_(std::move(cb)), threadId_(threadId) {
}

void Scheduler::coThread::reset() {
    co_ = nullptr;
    cb_ = nullptr;
    threadId_ = -1;
}

SchedulerSwitcher::SchedulerSwitcher(Scheduler* target) {
    caller_ = Scheduler::GetThis();
    if(target) {
        target->switchTo();
    }
}

SchedulerSwitcher::~SchedulerSwitcher() {
    if(caller_) {
        caller_->switchTo();
    }  
}




