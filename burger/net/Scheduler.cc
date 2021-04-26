#include "Scheduler.h"
#include "Processor.h"
#include "ProcessorThread.h"
#include "TimerQueue.h"
#include "Hook.h"
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

Scheduler::Scheduler(size_t threadNum) 
    : threadNum_(threadNum),
    timerQueue_(util::make_unique<TimerQueue>()) {
    assert(threadNum_ > 0);
    DEBUG("Scheduler ctor");
    assert(Processor::GetProcesserOfThisThread() == nullptr);
}

Scheduler::~Scheduler() {
    stop();
    DEBUG("Scheduler dtor");
}

void Scheduler::start() {
    if(running_) return;
    if(mainProc_ == nullptr) {
        mainProc_ = util::make_unique<Processor>(this);
        workProcVec_.push_back(mainProc_.get());
    }
    for(size_t i = 0; i < threadNum_ - 1; i++) {
        auto procThrd = std::make_shared<ProcessThread>(this);
        workThreadVec_.push_back(procThrd);
        workProcVec_.push_back(procThrd->startProcess());
    }
    DEBUG("timer thread started");
    // 能不能不单开timerThread
    // timerThread_ = std::make_shared<ProcessThread>(this);
    // timerProc_ = timerThread_->startProcess();
    // timerProc_->addTask([&]() {
    //     while(true) { // todo : check
    //         timerQueue_->dealWithExpiredTimer();
    //     }
    // }, "timer"); 
    running_ = true;
    cv_.notify_one();  // todo : 无其他线程，有影响吗
    mainProc_->run(); 
    DEBUG("Schedular start");
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
    for(const auto& proc : workProcVec_) {
        proc->stop();
    }
    // timerProc_->stop();
    // 如果在scheduler线程join，会自己join自己，导致dead lock error
    if(isHookEnable()) {
        std::thread joinThrd = std::thread{&Scheduler::joinThread, this};
        joinThrd.detach();
    } else {
        joinThread();
    }
}

void Scheduler::addTask(const Coroutine::Callback& task, std::string name) {
    Processor* proc = pickOneProcesser();
    assert(proc != nullptr);
    proc->addTask(task, name);
}

TimerId Scheduler::runAt(Coroutine::ptr co, Timestamp when) {
    // Processor* proc = Processor::GetProcesserOfThisThread();
    // if(proc == nullptr) {
    //     proc = pickOneProcesser();
    // } 
    Processor* proc = pickOneProcesser();
    return timerQueue_->addTimer(co, proc, when);
}

TimerId Scheduler::runAfter(Coroutine::ptr co, double delay) {
    Timestamp when = Timestamp::now() + delay;
    return runAt(co, when);
}

TimerId Scheduler::runEvery(Coroutine::ptr co, double interval) {
    // Processor* proc = Processor::GetProcesserOfThisThread();
    // if(proc == nullptr) {
    //     proc = pickOneProcesser();
    // } 
    Processor* proc = pickOneProcesser();
    Timestamp when = Timestamp::now() + interval; 
    return timerQueue_->addTimer(co, proc, when, interval);
}

void Scheduler::cancel(TimerId timerId) {
    return timerQueue_->cancel(timerId);
}

Processor* Scheduler::pickOneProcesser() {
    std::lock_guard<std::mutex> lock(mutex_);
    if(mainProc_ == nullptr) {
        mainProc_ = util::make_unique<Processor>(this);
        workProcVec_.push_back(mainProc_.get());
        return mainProc_.get();
    }
    static size_t index = 0;
    assert(index < workProcVec_.size());
    Processor* proc = workProcVec_[index++];
    index = index % workProcVec_.size();
    return proc;
}

void Scheduler::joinThread() {
    if(thread_.joinable()) thread_.join();
    for(auto thrd : workThreadVec_) {
        thrd->join();
    }
    // timerThread_->join();
    quit_ = true;
    quitCv_.notify_one();
    DEBUG("Thread Join");
}



    




