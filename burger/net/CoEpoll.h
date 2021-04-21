#ifndef COEPOLL_H
#define COEPOLL_H

#include <boost/noncopyable>
#include <vector>
#include <sys/epoll.h>
#include <map>

namespace burger {
namespace net {

class Coroutine;
class CoEpoll : boost::noncopyable {
public:
    CoEpoll(Processor* proc);
	// void updateEvent(int fd, int events, std::shared_ptr<Coroutine> co);
	// void removeEvent(int fd);

    void wait(int timeoutMs);
	bool isEpolling() { return isEpolling_; }
	void setEpolling(bool epolling) { isEpolling_ = epolling; }
private:
    bool isEpolling_;
    static const int kInitEventListSize = 16;
    std::vector<struct epoll_event> eventList_;
    std::map<int, std::shared<Coroutine> > coMap_; // todo：check 智能指针生命周期
    Processor* proc_;
    int epollFd_;
};

} // namespace net

} // namespace burger




#endif // COEPOLL_H