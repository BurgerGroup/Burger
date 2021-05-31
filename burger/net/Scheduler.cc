#include "Scheduler.h"
#include "Processor.h"
#include "ProcessorThread.h"
#include "CoTimerQueue.h"
#include "Hook.h"
#include "Timer.h"
#include "burger/base/Log.h"
#include "burger/base/Util.h"

#include <signal.h>
namespace {
#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe {
public:
    IgnoreSigPipe() {
        ::signal(SIGPIPE, SIG_IGN);
        TRACE("Ignore SIGPIPE");
    }
};
#pragma GCC diagnostic error "-Wold-style-cast"
IgnoreSigPipe initObj;

}  // namespace

using namespace burger;
using namespace burger::net;

Scheduler::Scheduler() {
    DEBUG("Scheduler ctor");
    assert(Processor::GetProcesserOfThisThread() == nullptr);
}

Scheduler::~Scheduler() {
    stop();
    DEBUG("Scheduler dtor");
}

// before start to call 
void Scheduler::setWorkProcNum(size_t workProcNum) {
    if(workProcNum > 100) {
        ERROR("set too many processors");
        workProcNum_ = 0;
    }
    workProcNum_ = workProcNum;
}

void Scheduler::start() {
    if(running_) return;

    std::unique_ptr<Processor> mainProc = util::make_unique<Processor>(this);
    mainProc_ = mainProc.get();

    for(size_t i = 0; i < workProcNum_; i++) {
        auto procThrd = util::make_unique<ProcessThread>(this);
        workProcVec_.push_back(procThrd->startProcess());
        workThreadVec_.push_back(std::move(procThrd));
    }
    running_ = true;
    cv_.notify_one();  // todo : 无其他线程，有影响吗
    mainProc->run();  // startAsync 在此处线程函数已执行结束
}

void Scheduler::startAsync() {
    if(running_) return;
    thread_ = std::thread{&Scheduler::start, this};
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]{ return running_ == true; });
    }
}

void Scheduler::wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    quitCv_.wait(lock, [this] { return quit_ == true; });
}

void Scheduler::stop() {
    if(!running_) return;
    running_ = false;
    mainProc_->stop();
    mainProc_ = nullptr;
    for(const auto& proc : workProcVec_) {
        proc->stop();
    }
    // 如果在scheduler线程join，会自己join自己，导致dead lock error
    if(isHookEnable()) {
        std::thread joinThrd = std::thread{&Scheduler::joinThread, this};
        joinThrd.detach();
    } else {
        joinThread();
    }
}

void Scheduler::addTask(const Coroutine::Callback& task, const std::string& name) {
    Processor* proc = pickOneWorkProcessor();
    assert(proc != nullptr);
    proc->addPendingTask(task, name);
}

void Scheduler::addMainTask(const Coroutine::Callback& task, const std::string& name) {
    Processor* proc = getMainProcessor();
    assert(proc != nullptr);
    proc->addPendingTask(task, name);
}

void Scheduler::distributeTask(const Coroutine::Callback& task, const std::string& name) {
    if(workProcVec_.empty()) {  
        mainProc_->addPendingTask(task, name);
    }else {
        for(auto& proc : workProcVec_) {
            proc->addPendingTask(task, name);
        }
    }
}

TimerId Scheduler::runAt(Timestamp when, Coroutine::ptr co) {
    Processor* proc = pickOneWorkProcessor();
    return proc->getTimerQueue()->addTimer(co, when);
}

TimerId Scheduler::runAfter(double delay, Coroutine::ptr co) {
    Timestamp when = Timestamp::now() + delay;
    return runAt(when, co);
}

TimerId Scheduler::runEvery(double interval, Coroutine::ptr co) {
    Processor* proc = pickOneWorkProcessor();
    Timestamp when = Timestamp::now() + interval; 
    return proc->getTimerQueue()->addTimer(co, when, interval);
}

TimerId Scheduler::runAt(Timestamp when, TimerCallback cb, const std::string& name) {
    Processor* proc = pickOneWorkProcessor();
    return proc->getTimerQueue()->addTimer(cb, name, when);
}

TimerId Scheduler::runAfter(double delay, TimerCallback cb, const std::string& name) {
    Timestamp when = Timestamp::now() + delay;
    return runAt(when, cb, name);
}

TimerId Scheduler::runEvery(double interval, TimerCallback cb, const std::string& name) {
    Processor* proc = pickOneWorkProcessor();
    Timestamp when = Timestamp::now() + interval; 
    return proc->getTimerQueue()->addTimer(cb, name, when, interval);
}

void Scheduler::cancel(TimerId timerId) {
    Processor* proc = timerId.timer_->getProcessor();
    return proc->getTimerQueue()->cancel(timerId);
}

size_t Scheduler::getWorkProcNum() {
    if(workProcVec_.empty()) return 1;
    return workProcNum_;
}

// 如果只有一个线程，那么返回mainProc, 否则返回工作Proc
Processor* Scheduler::pickOneWorkProcessor() {
    Processor* proc = mainProc_;
    static size_t index = 0;
    // std::lock_guard<std::mutex> lock(mutex_); 
    // 此处 workProcVec_ 都是读者，不用加锁
    if(workProcVec_.empty() && mainProc_ == nullptr) {
        CRITICAL("start scheduler first");
    }
    if(!workProcVec_.empty()) {
        // round robin
        assert(index < workProcVec_.size());
        proc = workProcVec_[index++];
        index = index % workProcVec_.size();
    }
    return proc;
}

void Scheduler::joinThread() {
    if(thread_.joinable()) thread_.join();
    for(auto& thrd : workThreadVec_) {
        thrd->join();
    }
    quit_ = true;
    quitCv_.notify_one();
    DEBUG("Thread Join");
}



    




