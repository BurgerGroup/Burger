#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <cassert>
#include <boost/noncopyable.hpp>

namespace burger {
namespace net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : boost::noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    EventLoopThreadPool(EventLoop* baseLoop);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    // valid after calling start(),round robin
    EventLoop* getNextLoop();

private:
    EventLoop* baseLoop_;  // acceptor loop
    bool started_;
    int numThreads_;
    int next_;  // next connection's EventLoop obj id
    std::vector<std::unique_ptr<EventLoopThread> > threadList_;
    std::vector<EventLoop *> loopList_;  // 都是栈上对象
};

} // namespace net

} // namespace burge



#endif // EVENTLOOPTHREADPOOL_H