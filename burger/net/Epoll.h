#ifndef EPOLL_H
#define EPOLL_H

#include "Channel.h"

#include "burger/base/Timestamp.h"
#include <boost/noncopyable.hpp>
#include <vector>
#include <memory>
#include <map>

#include <sys/epoll.h>

namespace burger {
namespace net {
class EventLoop;

class Epoll : boost::noncopyable {
public:

    Epoll(EventLoop* loop);
    ~Epoll();

    Timestamp wait(int timeoutMs, std::vector<Channel*>& activeChannels);
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel) const;

    void assertInLoopThread() const;

private:
    void fillActiveChannels(int numEvents, 
                std::vector<Channel*>& activeChannels) const;
    void update(int operation, Channel* channel);
    static std::string operationToString(int op);
private:
    static const int kInitEventListSize = 16;
    std::map<int, Channel*> channelMap_;
    EventLoop* ownerLoop_;
    int epollFd_;
    std::vector<struct epoll_event> eventList_;
};
} // namespace net

} // namespace burger

#endif // EPOLL_H