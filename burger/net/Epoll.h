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

    Timestamp wait(int timeoutMs, std::vector<ChannelPtr>& activeChannels);
    void updateChannel(ChannelPtr channel);
    void removeChannel(ChannelPtr channel);
    bool hasChannel(ChannelPtr channel) const;

    void assertInLoopThread() const;

private:
    void fillActiveChannels(int numEvents, 
                std::vector<ChannelPtr>& activeChannels) const;
    void update(int operation, ChannelPtr channel);
    static std::string operationToString(int op);
private:
    static const int kInitEventListSize = 16;
    std::map<int, ChannelPtr> channelMap_;
    EventLoop* ownerLoop_;
    int epollFd_;
    std::vector<struct epoll_event> eventList_;
};
} // namespace net

} // namespace burger

#endif // EPOLL_H