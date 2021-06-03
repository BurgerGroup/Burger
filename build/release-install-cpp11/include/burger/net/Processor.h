#ifndef PROCESSER_H
#define PROCESSER_H

#include <boost/noncopyable.hpp>
#include <mutex>
#include <memory>
#include <functional>
#include <queue>
#include <unordered_map>
#include <vector>
#include "burger/base/Coroutine.h"
#include "CoEpoll.h"
#include "burger/base/MpscQueue.h"

namespace burger {
namespace net {

class CoEpoll;
class Scheduler;
class CoTimerQueue;
class CoTcpConnection;
class Processor : boost::noncopyable {
public:
    using ptr = std::shared_ptr<Processor>;
    using task = std::pair<Coroutine::Callback, std::string>;

    Processor(Scheduler* scheduler);
    virtual ~Processor();

    virtual void run();
    void stop();
    bool stoped() { return stop_; }
    size_t getLoad() const { return load_; }
    size_t getCreateTimes() const { return totolCoCreateTimes_; }  // for test
    Scheduler* getScheduler() { return scheduler_; }
    Coroutine::ptr resetAndGetCo(const Coroutine::Callback& cb, const std::string& name);

    void addTask(Coroutine::ptr co);
    void addTask(const Coroutine::Callback& cb, const std::string& name = "");
    // 跨线程调用addTask
    void addPendingTask(const Coroutine::Callback& cb, const std::string& name = "");
    void updateEvent(int fd, int events);
    void removeEvent(int fd);

    static Processor* GetProcesserOfThisThread();
    void assertInProcThread();
    bool isInProcThread() const;

    CoTimerQueue* getTimerQueue() { return timerQueue_.get(); }

    void wakeupEpollCo();
    ssize_t consumeWakeUp();

    void addToConnMap(int fd, std::shared_ptr<CoTcpConnection> conn);

private: 
    void addPendingTasksIntoQueue();
    void abortNotInProcThread();

    bool stop_ = false;
    bool addingPendingTasks_ = false;
    size_t load_ = 0;
    size_t totolCoCreateTimes_ = 0;
    std::mutex mutex_;
    Scheduler* scheduler_;
    CoEpoll epoll_;
    // std::unique_ptr<CoEpoll> epoll_; // https://stackoverflow.com/questions/20268482/binding-functions-with-unique-ptr-arguments-to-stdfunctionvoid
    std::unique_ptr<CoTimerQueue> timerQueue_;
    int wakeupFd_;
    std::queue<Coroutine::ptr> runnableCoQue_;
    std::queue<Coroutine::ptr> idleCoQue_;
    std::vector<task> pendingTasks_;
    const pid_t threadId_;  // 当前对象所属线程Id
    std::unordered_map<int, std::shared_ptr<CoTcpConnection> > connMap_;  

};



} // namespace net

} // namespace burger



#endif // PROCESSER_H
