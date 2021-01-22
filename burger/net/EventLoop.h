#ifndef EVENTLOOP_H
#define EVENTLOOP_H
#include <boost/noncopyable.hpp>
#include <memory>
#include <thread>
#include <iostream>
#include "burger/base/Util.h"
#include "burger/base/Log.h"


namespace burger {
namespace net {
class EventLoop : boost::noncopyable, 
                public std::enable_shared_from_this<EventLoop>  {
public:
    using ptr = std::shared_ptr<EventLoop>;
    EventLoop();
    ~EventLoop() = default;
    static ptr create();
    void loop();
    void assertInLoopThread();
    bool isInLoopThread() const;
    static ptr getEventLoopOfCurrentThread();
private:
    void init();
    void abortNotInLoopThread();
private:
    bool looping_; // atomic
    pid_t threadId_;  // 当前对象所属线程ID
};


} // namespace net

} // namespace burge 


#endif // EVENTLOOP_H