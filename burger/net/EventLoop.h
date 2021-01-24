#ifndef EVENTLOOP_H
#define EVENTLOOP_H
#include <boost/noncopyable.hpp>
#include <memory>
#include <thread>
#include <iostream>
#include "burger/base/Util.h"
#include "burger/base/Log.h"
#include "Channel.h"

namespace burger {
namespace net {

class Channel;

class EventLoop : boost::noncopyable, 
                public std::enable_shared_from_this<EventLoop>  {
public:
    using ptr = std::shared_ptr<EventLoop>;
    EventLoop();
    ~EventLoop() = default;
    static ptr create();
    void loop();
    void assertInLoopThread();
    bool isInLoopThread() const;
    static ptr getEventLoopOfCurrentThread();

    void wakeup();
    void updateChannel(Channel::ptr channel);
    void removeChannel(Channel::ptr channel);
    bool hasChannel(Channel::ptr channel);
private:
    void init();
    void abortNotInLoopThread();
private:
    bool looping_; // atomic
    const pid_t threadId_;  // 当前对象所属线程ID
    std::unique_ptr<Epoll> epoll_;
    Channel::ptrList activeChannels_;
    Channel::ptr currentActiveChannel_;
};


} // namespace net

} // namespace burge 


#endif // EVENTLOOP_H