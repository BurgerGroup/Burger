#ifndef PROCESSER_H
#define PROCESSER_H

#include <boost/noncopyable.hpp>
#include <mutex>
#include <memory>
#include <functional>
#include <queue>
#include <vector>
#include "burger/base/Coroutine.h"
#include "CoEpoll.h"

namespace burger {
namespace net {

class CoEpoll;
class Scheduler;
class CoTimerQueue;

class Processor : boost::noncopyable {
public:
    using ptr = std::shared_ptr<Processor>;
    using task = std::pair<Coroutine::Callback, std::string>;

    Processor(Scheduler* scheduler);
    virtual ~Processor();

    virtual void run();
    void stop();
    bool stoped() { return stop_; }
    size_t getLoad() { return load_; }
    Scheduler* getScheduler() { return scheduler_; }
    
    void addTask(Coroutine::ptr co, const std::string& name = "");
    void addTask(const Coroutine::Callback& cb, const std::string& name = "");
    void addPendingTask(const Coroutine::Callback& cb, const std::string& name = "");
    void updateEvent(int fd, int events, Coroutine::ptr co = nullptr);
    void removeEvent(int fd);

    static Processor* GetProcesserOfThisThread();

    CoTimerQueue* getTimerQueue() { return timerQueue_.get(); }


    void wakeupEpollCo();
    ssize_t consumeWakeUp();
private: 
    void addPendingTasksIntoQueue();
    std::shared_ptr<Coroutine> mainCo_;
    bool stop_ = false;
    bool addingPendingTasks_ = false;
    size_t load_ = 0;
    std::mutex mutex_;
    Scheduler* scheduler_;
    CoEpoll epoll_;
    // std::unique_ptr<CoEpoll> epoll_; // https://stackoverflow.com/questions/20268482/binding-functions-with-unique-ptr-arguments-to-stdfunctionvoid
    std::unique_ptr<CoTimerQueue> timerQueue_;
    int wakeupFd_;
    std::queue<Coroutine::ptr> runnableCoQue_;
    std::queue<Coroutine::ptr> idleCoQue_;
    std::vector<task> pendingTasks_;

};



} // namespace net

} // namespace burger



#endif // PROCESSER_H
