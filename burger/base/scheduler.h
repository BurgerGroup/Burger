#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <mutex>
#include <memory>
#include <vector>
#include <list>
#include <thread>
#include <mutex>
#include <functional>
#include <boost/noncopyable.hpp>
#include "coroutine.h"

namespace burger {

class Coroutine;

// 协程调度器，N-M，内部一个线程池i，支持协程在线程池里切换
class Scheduler {
public:
    using ptr = std::shared_ptr<Scheduler>;
    using Callback = std::function<void()>;

    Scheduler(size_t threadNum = 1, bool useCaller = true, const std::string& name = "");
    virtual ~Scheduler();
    const std::string& getName() const { return name_;}
    static Scheduler* GetThis();
    static Coroutine* GetSchedCo();    

    // for threadpool
    void start();
    void stop();

    // threadId 协程执行的线程id,-1标识任意线程
    template<class CoOrCb>
    void schedule(CoOrCb cocb, int threadId = -1);

    template<class InputIterator>
    void schedule(InputIterator begin, InputIterator end);  // 批量调度协程

    void switchTo(pid_t threadId = -1);
    std::ostream& dump(std::ostream& os);
protected:
    virtual void tickle();  // 通知协程调度器有任务了
    void run();  // 协程调度函数
    virtual bool stopping();  // 返回是否可以停止
    virtual void idle();  // 协程无任务可调度时执行idle协程
    void setThis();  // 设置当前的协程调度器

    bool hasIdleThreads() { return idleThreadCount_ > 0;}  // 是否有空闲线程
private:
    template<class CoOrCb>
    bool scheduleNoLock(CoOrCb cocb, pid_t threadId);
private:
    // 协程/函数/线程组
    struct coThread {
        Coroutine::ptr co_;
        std::function<void()> cb_; // 协程执行函数
        pid_t threadId_;

        // todo:
        coThread(const Coroutine::ptr& co, pid_t threadId);
        coThread(Coroutine::ptr&& co, pid_t threadId);
        coThread(const Callback& cb, pid_t threadId);
        coThread(Callback&& cb, pid_t threadId);
        coThread() :threadId_(-1) {}
        void reset();  // 重置数据
    };
private:
    std::mutex mutex_;
    std::vector<std::thread> threadVec_;  // 线程池
    std::list<coThread> coList_;  // 待执行的协程队列
    Coroutine::ptr rootCo_;  // useCaller为true时有效, 调度协程
    std::string name_;
    
protected:
    std::vector<pid_t> threadIdList_;  // 协程下的线程id数组
    size_t threadCount_ = 0;
    std::atomic<size_t> activeThreadCount_ = {0};
    std::atomic<size_t> idleThreadCount_ = {0};
    bool stopping_ = true;  // 是否正在停止
    bool autoStop_ = false;  // 是否自动停止
    pid_t rootThreadId_ = 0; // 主线程id(useCaller)
};


class SchedulerSwitcher : public boost::noncopyable {
public:
    SchedulerSwitcher(Scheduler* target = nullptr);
    ~SchedulerSwitcher();
private:
    Scheduler* caller_;
};


/// implement 
template<class CoOrCb>
void Scheduler::schedule(CoOrCb cocb, pid_t threadId) {
    bool needTickle = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        needTickle = scheduleNoLock(cocb, threadId);
    }

    if(needTickle) {
        tickle();
    }
}

template<class InputIterator>
void Scheduler::schedule(InputIterator begin, InputIterator end) {
    bool needTickle = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        while(begin != end) {
            // todo check
            needTickle = scheduleNoLock(*begin, -1) || needTickle;
            ++begin;
        }
    }
    if(needTickle) {
        tickle();
    }
}

template<class CoOrCb>
bool Scheduler::scheduleNoLock(CoOrCb cocb, pid_t threadId) {
    bool needTickle = coList_.empty();
    coThread coThrd(cocb, threadId);
    if(coThrd.co_ || coThrd.cb_) {
        coList_.push_back(coThrd);
    }
    return needTickle;
}

} // namespace burger



#endif // SCHEDULER_H