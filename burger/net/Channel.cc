#include "Channel.h"


using namespace burger;
using namespace burger::net;

void Channel::update() {
    addedToLoop_ = true;
    loop_->
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
    if (event & EPOLLNVAL)
        oss << "NVAL ";

    return oss.str();
}


const uint32_t Channel::kNoneEvent = 0;
const uint32_t Channel::kReadEvent = EPOLLIN | EPOLLPRI;  // EPOLLPRI 外带数据
const uint32_t Channel::kWriteEvent = EPOLLOUT;


