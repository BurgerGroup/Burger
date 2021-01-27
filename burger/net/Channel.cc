#include "EventLoop.h"

using namespace burger;
using namespace burger::net;

Channel::Channel(EventLoop* loop, int fd):
    loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    status_(-1),
    tied_(false),
    eventHandling_(false),
    addedToLoop_(false) {
}

Channel::~Channel() {
    assert(!eventHandling_);
    assert(!addedToLoop_);
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
            handleEventWithGuard(receiveTime);
        }
    } else {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::update() {
    addedToLoop_ = true;
    loop_->updateChannel(this);
}

void Channel::remove() {
    assert(isNoneEvent());
    addedToLoop_ = false;
    loop_->removeChannel(this);
}

// TODO
void Channel::handleEventWithGuard(Timestamp receiveTime) {
    eventHandling_ = true;
    TRACE("{}", reventsToString());
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if (logHup_) {
            WARN("fd = {}  Channel::handle_event POLLHUP", fd_);
        }
        if (closeCallback_) closeCallback_();
    }

    if (revents_ & EPOLLERR) {
        if (errorCallback_) errorCallback_();
    }
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (readCallback_) readCallback_(receiveTime);
    }
    if (revents_ & EPOLLOUT) {
        if (writeCallback_) writeCallback_();
    }
    eventHandling_ = false;
}

std::string Channel::eventsToString() const {
    return eventsToString(fd_, events_);
}


std::string Channel::reventsToString() const {
    return eventsToString(fd_, revents_);
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


const uint32_t Channel::kNoneEvent = 0;
const uint32_t Channel::kReadEvent = EPOLLIN | EPOLLPRI;  // EPOLLPRI 外带数据
const uint32_t Channel::kWriteEvent = EPOLLOUT;


