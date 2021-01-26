#ifndef EPOLL_H
#define EPOLL_H

#include "EventLoop.h"
#include "burger/base/Timestamp.h"
#include <boost/noncopyable.hpp>
#include <vector>
#include <memory>
#include <map>

#include <sys/epoll.h>

namespace burger {
namespace net {

class EventLoop;
class Channel;

class Epoll : boost::noncopyable {
public:
    Epoll(EventLoop::ptr loop);
    ~Epoll();

    Timestamp wait(int timeoutMs, Channel::ptrList& activeChannels);
    void updateChannel(Channel::ptr channel);
    void removeChannel(Channel::ptr channel);
    bool hasChannel(Channel::ptr channel) const;

    void assertInLoopThread() const;

private:
    void fillActiveChannels(int numEvents, 
                Channel::ptrList activeChannels) const;
    void update(int operation, Channel::ptr channel);
    static std::string operationToString(int op);
private:
    static const int kInitEventListSize = 16;
    std::map<int, Channel::ptr> channelMap_;
    EventLoop::ptr ownerLoop_;
    int epollFd_;
    std::vector<struct epoll_event> eventList_;
};
} // namespace net

} // namespace burger

#endif // EPOLL_H