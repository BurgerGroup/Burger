#include "CoEpoll.h"
#include "SocketsOps.h"
#include "burger/base/Log.h"

using namespace burger;
using namespace burger::net;

CoEpoll::CoEpoll(Processor* proc)
    : isEpolling_(false),
    eventList_(kInitEventListSize),
    proc_(proc),
    epollFd_(::epoll_create1(EPOLL_CLOEXEC)),
    {
    if(epollFd_ < 0) {
        CRITIAL("CoEpoll::CoEpoll error");
    }
}

void CoEpoll::wait(int timeoutMs) {
    while(!proc_.stoped()) {
        int numEvents = ::epoll_wait(epollFd_, eventList_.data(),
                    static_cast<int>(eventList_.size()), timeoutMs);
        int savedErrno = errno;
        if(numEvents > 0) {
            TRACE("{} events happended", numEvents);
        }
    }
}

// void CoEpoll::updateEvent(int fd, int events, std::shared_ptr<Coroutine> co) {
//     assert(co != nullptr);
//     auto it = coMap_.find(fd);
//     if(it == coMap_.end()) {   // add
//         struct 
//     }  
// }
