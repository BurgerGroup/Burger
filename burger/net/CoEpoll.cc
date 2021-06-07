#include "CoEpoll.h"
#include "SocketsOps.h"
#include "Processor.h"
#include "burger/base/Log.h"

using namespace burger;
using namespace burger::net;

const uint32_t CoEpoll::kNoneEvent = 0;
const uint32_t CoEpoll::kReadEvent = EPOLLIN | EPOLLPRI;
const uint32_t CoEpoll::kWriteEvent = EPOLLOUT;

CoEpoll::CoEpoll(Processor* proc)
    : isEpolling_(false),
    eventList_(kInitEventListSize),
    proc_(proc),
    epollFd_(::epoll_create1(EPOLL_CLOEXEC)) {
    if(epollFd_ < 0) {
        CRITICAL("CoEpoll::CoEpoll error");
    }
}

CoEpoll::~CoEpoll() {
    ::close(epollFd_);
    // 关注的都进去
    for(auto it = coMap_.begin(); it != coMap_.end(); it++) {
        it->second->resume();
    }
    DEBUG("CoEpoll dtor");
}

void CoEpoll::updateEvent(int fd, int events) {
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.data.fd = fd;
    event.events = events;
    auto it = coMap_.find(fd);
    if(it == coMap_.end()) {  // not find, add current co 
        auto co = Coroutine::GetCurCo();

        DEBUG("add event {} - {}", fd, co->getName());
        coMap_.insert({fd, co});
        co->setFd(fd);
        int ret = epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event);
        if(ret == 1) {
            ERROR("CoEpoll add error");
        }
    } else {  // find mod
        int ret = epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event);
        if(ret == -1) {
            ERROR("CoEpoll mod error");
        }
    }
}

void CoEpoll::removeEvent(int fd) {
    auto it = coMap_.find(fd);
    if(it == coMap_.end()) {
        DEBUG("remove event but not found");
        return; // 未找到这个fd    
    } else {
        DEBUG("remove event {} - {}", fd, it->second->getName());
        auto co = coMap_[fd];
        co->setFd(-1);
        coMap_.erase(fd);
    }
    int ret = ::epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, nullptr);
    if(ret == -1) {
        ERROR("CoEpoll removeEvent error");
    }
}

void CoEpoll::poll(int timeoutMs) {
    while(!proc_->stoped()) {
        TRACE("fd total count {}", coMap_.size());
        isEpolling_ = true;
        int numEvents = ::epoll_wait(epollFd_, eventList_.data(),
                    static_cast<int>(eventList_.size()), timeoutMs);
        isEpolling_ = false;
        int savedErrno = errno;
        if(numEvents > 0) {
            TRACE("{} events happended", numEvents);
            for(int i = 0; i < numEvents; i++) {
                auto co = coMap_[eventList_[i].data.fd];
                assert(co != nullptr);
                
                proc_->addTask(co);
            }
            if(static_cast<size_t>(numEvents) == eventList_.size()) {
                eventList_.resize(eventList_.size() * 2);
            }
        } else if(numEvents == 0) {
            TRACE("Nothing happened ....");
        } else {
            // 如果不是EINTR信号，就把错误号保存下来，并且输入到日志中
            if(savedErrno != EINTR) {
                errno = savedErrno;
                ERROR("CoEpoll:wait() error");
            }
        }
        Coroutine::Yield();
    }
}

