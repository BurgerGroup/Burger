#include "Channel.h"

using namespace burger;
using namespace burger::net;

void Channel::update() {
    addedToLoop_ = true;
    loop_->updateChannel(shared_from_this());
}

std::string Channel::eventsToString() const {
    return eventsToString(fd_, events_);
}


std::string Channel::reventsToString() const {
    return eventsToString(fd_, revents_);
}

void Channel::remove() {
    assert(isNoneEvent());
    addedToLoop_ = false;
    loop_->removeChannel(shared_from_this());
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


