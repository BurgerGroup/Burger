#ifndef EVENTLOOP_H
#define EVENTLOOP_H
#include <boost/noncopyable.hpp>
#include <memory>
#include <thread>
#include <mutex>
#include <iostream>
#include <vector>
#include "burger/base/Util.h"
#include "burger/base/Log.h"
#include "burger/base/Timestamp.h"
#include <sys/eventfd.h>
#include "TimerId.h"

#include "SocketsOps.h"
#include "Callbacks.h"

namespace burger {
namespace net {


class Epoll;
class TimerQueue;
class Channel;

class EventLoop : boost::noncopyable {
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();
    Timestamp epollWaitRetrunTime() const { return epollWaitReturnTime_; }
    int64_t iteration() const { return iteration_; }
    // 在主循环中进行， safe to call from other threads
    void runInLoop(Functor func);
    // 插入主循环任务队列, safe to call from other threads
    void queueInLoop(Functor func);
    size_t queueSize() const;
    // timers , safe ti call from other threads
    TimerId runAt(Timestamp time, TimerCallback timercb);
    TimerId runAfter(double delay, TimerCallback timercb);
    TimerId runEvery(double interval, TimerCallback timercb);
    void cancel(TimerId timerId); 

    void assertInLoopThread();
    bool isInLoopThread() const;
    static EventLoop* getEventLoopOfCurrentThread();

    void wakeup();
    void updateChannel(Channel* channel); // 从epoll添加或更新channel
    void removeChannel(Channel* channel); // 从epoll里移除channel
    bool hasChannel(Channel* channel);
private:
    void init();
    void abortNotInLoopThread();
    void handleWakeupFd(); 
    void printActiveChannels() const;   // for DEBUG
    void doPendingFunctors();
private:
    bool looping_; // atomic   
    std::atomic<bool> quit_;  // linux下bool也是atomic的
    bool eventHandling_;  // atomic
    bool callingPendingFunctors_; // atomic
    int64_t iteration_;
    const pid_t threadId_;  // 当前对象所属线程ID
    Timestamp epollWaitReturnTime_;
    std::unique_ptr<Epoll> epoll_;
    std::unique_ptr<TimerQueue> timerQueue_;
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;
    std::vector<Channel*> activeChannels_;
    Channel* currentActiveChannel_;
    mutable std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;
};


} // namespace net

} // namespace burge 


#endif // EVENTLOOP_H