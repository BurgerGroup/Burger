#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
using namespace burger;
using namespace burger::net;

Epoll::Epoll(EventLoop* loop) :
    ownerLoop_(loop), 
    epollFd_(::epoll_create1(EPOLL_CLOEXEC)),
    eventList_(kInitEventListSize) {
    if(epollFd_ < 0) {
        CRITICAL("Epoll:Epoll error");
    }
}

Epoll::~Epoll() {
    ::close(epollFd_);   
}

Timestamp Epoll::wait(int timeoutMs, std::vector<Channel*>& activeChannels) {
    TRACE("fd total count {}", channelMap_.size());
    int numEvents = ::epoll_wait(epollFd_, eventList_.data(),
                static_cast<int>(eventList_.size()), timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if(numEvents > 0) {
        TRACE("{} events happended", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if(static_cast<size_t>(numEvents) == eventList_.size()) {
            eventList_.resize(eventList_.size() * 2);
        }
    } else if(numEvents == 0) {
        TRACE("Nothing happend...");
    } else {
        if(savedErrno != EINTR) {
            errno = savedErrno;
            ERROR("Epoll::wait() error");
        }
    }
    return now;
}

void Epoll::updateChannel(Channel* channel) {
    assertInLoopThread();
    const Channel::Status status = channel->getStatus();
    int fd = channel->getFd();
    TRACE("fd = {}, events = {}, stauts = {}", 
        fd, channel->getEvents(), Channel::statusTostr(status)); 
    if(status == Channel::Status::kEnumNew || status == Channel::Status::kEnumDismissed) {
        // a new one , add with EPOLL_CTL_ADD
        if(status == Channel::Status::kEnumNew) {
            assert(channelMap_.find(fd) == channelMap_.end());
            channelMap_[fd] = channel;
        } else { // status = kEnumDissmiissed
            assert(channelMap_.find(fd) != channelMap_.end());
            assert(channelMap_[fd] == channel);
        }
        channel->setStatus(Channel::Status::kEnumAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {  // kAdded
        // update existing one with EPOLL_CTL_MOD/DEL
        assert(channelMap_.find(fd) != channelMap_.end());
        assert(channelMap_[fd] == channel);
        assert(status == Channel::Status::kEnumAdded);
        if(channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->setStatus(Channel::Status::kEnumDismissed);
            // TODO：为什么不马上从map里erase掉
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }

}

void Epoll::removeChannel(Channel* channel)  {
    assertInLoopThread();
    int fd = channel->getFd();
    TRACE("fd = {} ", fd);
    assert(channelMap_.find(fd) != channelMap_.end());
    assert(channelMap_[fd] == channel);
    assert(channel->isNoneEvent());
    Channel::Status status = channel->getStatus();
    assert(status == Channel::Status::kEnumAdded || status == Channel::Status::kEnumDismissed);
    size_t n = channelMap_.erase(fd);  // TODO : why use map other than unordered_map
    assert(n == 1);
    if(status == Channel::Status::kEnumAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setStatus(Channel::Status::kEnumNew);
}

void Epoll::fillActiveChannels(int numEvents, 
            std::vector<Channel*>& activeChannels) const {
    assert(static_cast<size_t>(numEvents) <= eventList_.size());
    for(int i = 0; i < numEvents; i++) {
        Channel* channel = static_cast<Channel*>(eventList_[i].data.ptr);
#ifndef NDEBUG
        int fd = channel->getFd();
        auto it = channelMap_.find(fd);
        assert(it != channelMap_.end());
        assert(it->second == channel);
#endif
        activeChannels.push_back(channel);
    }
}

void Epoll::update(int operation, Channel* channel) {
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = channel->getEvents();
    // TODO 这里需要check
    event.data.ptr = static_cast<void*>(channel);
    int fd = channel->getFd();
    TRACE("epoll_ctl op = {}, fd = {}, event = [{}]", 
        operationToString(operation), fd, channel->eventsToString());
    if(::epoll_ctl(epollFd_, operation, fd, &event) < 0) {
        if(operation == EPOLL_CTL_DEL) {
            ERROR("epoll_ctl op = {}, fd = {} ", 
                operationToString(operation), fd);
        } else {
            CRITICAL("epoll_ctl op = {}, fd = {} ", 
                operationToString(operation), fd);
        }
    }
}

std::string Epoll::operationToString(int op) {
    switch(op) {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            return "Unknown Operation";
    }
}

bool Epoll::hasChannel(Channel* channel) const {
    assertInLoopThread();
    auto it = channelMap_.find(channel->getFd());
    return it != channelMap_.end() && it->second == channel;
}

void Epoll::assertInLoopThread() const {
    ownerLoop_->assertInLoopThread();
}


