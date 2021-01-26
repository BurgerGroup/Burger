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

#include "SocketsOps.h"
#include "Channel.h"
#include "Epoll.h"

namespace burger {
namespace net {

class EventLoop : boost::noncopyable {
public:
    using ptr = std::shared_ptr<EventLoop>;
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();
    static ptr create();
    void loop();
    void quit();
    Timestamp epollWaitRetrunTime() const { return epollWaitReturnTime_; }
    int64_t iteration() const { return iteration_; }
    void assertInLoopThread();
    bool isInLoopThread() const;
    static EventLoop* getEventLoopOfCurrentThread();

    void wakeup();
    void updateChannel(ChannelPtr channel);
    void removeChannel(ChannelPtr channel);
    bool hasChannel(ChannelPtr channel);
private:
    void init();
    void abortNotInLoopThread();
    void handleRead();  //wake up
    void printActiveChannels() const;   // for DEBUG
    void doPendingFunctors();
private:
    bool looping_; // atomic
    std::atomic<bool> quit_;
    bool eventHandling_;  // atomic
    bool callingPendingFunctors_; // atomic
    int64_t iteration_;
    const pid_t threadId_;  // 当前对象所属线程ID
    Timestamp epollWaitReturnTime_;
    std::unique_ptr<Epoll> epoll_;
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;
    std::vector<ChannelPtr> activeChannels_;
    std::shared_ptr<Channel> currentActiveChannel_;
    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;
};


} // namespace net

} // namespace burge 


#endif // EVENTLOOP_H