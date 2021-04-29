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
    running_ = true;
    cv_.notify_one();  // todo : 无其他线程，有影响吗
    mainProc_->run(); 
    DEBUG("Schedular start() end");
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
    proc->addPendingTask(task, name);
}

TimerId Scheduler::runAt(Timestamp when, Coroutine::ptr co) {
    Processor* proc = pickOneProcesser();
    return timerQueue_->addTimer(co, proc, when);
}

TimerId Scheduler::runAfter(double delay, Coroutine::ptr co) {
    Timestamp when = Timestamp::now() + delay;
    return runAt(when, co);
}

TimerId Scheduler::runEvery(double interval, Coroutine::ptr co) {
    Processor* proc = pickOneProcesser();
    Timestamp when = Timestamp::now() + interval; 
    return timerQueue_->addTimer(co, proc, when, interval);
}

TimerId Scheduler::runAt(Timestamp when, TimerCallback cb) {
    Coroutine::ptr co = std::make_shared<Coroutine>(cb, "timer");
    return runAt(when, co);
}

TimerId Scheduler::runAfter(double delay, TimerCallback cb) {
    Coroutine::ptr co = std::make_shared<Coroutine>(cb, "timer");
    return runAfter(delay, co);
}

TimerId Scheduler::runEvery(double interval, TimerCallback cb) {
    Coroutine::ptr co = std::make_shared<Coroutine>(cb, "timer");
    return runEvery(interval, co);
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
    quit_ = true;
    quitCv_.notify_one();
    DEBUG("Thread Join");
}



    




