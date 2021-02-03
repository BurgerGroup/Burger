#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
using namespace burger;
using namespace burger::net;

Channel::Channel(EventLoop* loop, int fd):
    loop_(loop),
    fd_(fd),
    events_(0),
    status_(Status::kEnumNew),
    tied_(false),
    eventHandling_(false),
    addedToEpoll_(false) {
}

Channel::~Channel() {
    assert(!eventHandling_);
    assert(!addedToEpoll_);
    if (loop_->isInLoopThread()) {
        assert(!loop_->hasChannel(this));
    }
}

// TODO
void Channel::handleEvent(Timestamp receiveTime) {
    std::shared_ptr<void> guard;
    if(tied_) {
        guard = tie_.lock();
        if(guard) {
            TRACE("[6] usecount = {}", guard.use_count());
            handleEventWithGuard(receiveTime);
            TRACE("[12] usecount = {}", guard.use_count());
        }
    } else {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::update() {
    addedToEpoll_ = true;
    loop_->updateChannel(this);
}

void Channel::remove() {
    assert(isNoneEvent());
    addedToEpoll_ = false;
    loop_->removeChannel(this);
}

std::string Channel::statusTostr(Status status) {
    switch(status) {
#define XX(name) \
    case Status::name: \
        return #name; \
        break;
    
    XX(kEnumNew);
    XX(kEnumAdded);      
    XX(kEnumDismissed)
#undef XX
    default:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}

// TODO
void Channel::handleEventWithGuard(Timestamp receiveTime) {
    eventHandling_ = true;
    TRACE("{}", eventsToString());
    if ((events_ & EPOLLHUP) && !(events_ & EPOLLIN)) {
        if (logHup_) {
            WARN("fd = {}  Channel::handle_event POLLHUP", fd_);
        }
        if (closeCallback_) closeCallback_();
    }

    if (events_ & EPOLLERR) {
        if (errorCallback_) errorCallback_();
    }
    if (events_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (readCallback_) readCallback_(receiveTime);
    }
    if (events_ & EPOLLOUT) {
        if (writeCallback_) writeCallback_();
    }
    eventHandling_ = false;
}

std::string Channel::eventsToString() const {
    return eventsToString(fd_, events_);
}

std::string Channel::eventsToString(int fd, int event) {
    std::ostringstream oss;
    oss << fd << ": ";
    if (event & EPOLLIN)
        oss << "IN ";
    if (event & EPOLLPRI)
        oss << "PRI ";
    if (event & EPOLLOUT)
        oss << "OUT ";
    if (event & EPOLLHUP)
        oss << "HUP ";
    if (event & EPOLLRDHUP)
        oss << "RDHUP ";
    if (event & EPOLLERR)
        oss << "ERR ";
    return oss.str();
}

void Channel::tie(const std::shared_ptr<void>& obj) {
    tie_ = obj;  // tie_ is weak ptr, so not add 1 to use_count
    tied_ = true;
}


const uint32_t Channel::kNoneEvent = 0;
const uint32_t Channel::kReadEvent = EPOLLIN | EPOLLPRI;  // EPOLLPRI 外带数据
const uint32_t Channel::kWriteEvent = EPOLLOUT;


