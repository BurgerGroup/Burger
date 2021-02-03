#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H

#include <string>
#include <memory>
#include <vector>
#include <boost/noncopyable.hpp>

namespace burger {
namespace net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : boost::noncopyable {
public:
    

private:
    EventLoop* baseLoop_;  // acceptor loop
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;  // next connection's EventLoop obj id
    std::vector<std::unique_ptr<EventLoopThread> > threads_;
    std::vector<EventLoop*> loops_;
};

} // namespace net

} // namespace burge



#endif // EVENTLOOPTHREADPOOL_H