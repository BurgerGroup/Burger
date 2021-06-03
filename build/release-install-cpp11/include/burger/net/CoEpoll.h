#ifndef COEPOLL_H
#define COEPOLL_H

#include <boost/noncopyable.hpp>
#include <vector>
#include <sys/epoll.h>
#include <map>
#include <memory>
#include "burger/base/Coroutine.h"

namespace burger {
namespace net {

class Processor;

class CoEpoll : boost::noncopyable {
public:
    // https://stackoverflow.com/questions/9954518/stdunique-ptr-with-an-incomplete-type-wont-compile
    CoEpoll(Processor* proc);
    ~CoEpoll();
	void updateEvent(int fd, int events);
	void removeEvent(int fd);

    void poll(int timeoutMs);
	bool isEpolling() { return isEpolling_; }
	void setEpolling(bool epolling) { isEpolling_ = epolling; }
    static const uint32_t kNoneEvent;
    static const uint32_t kReadEvent;
    static const uint32_t kWriteEvent;

private:
    bool isEpolling_;
    static const int kInitEventListSize = 16;
    std::vector<struct epoll_event> eventList_;
    std::map<int, Coroutine::ptr> coMap_; // todo：check 智能指针生命周期
    Processor* proc_;
    int epollFd_;
};

} // namespace net

} // namespace burger




#endif // COEPOLL_H