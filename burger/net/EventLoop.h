/*
 * @Author: Shiyu Yi
 * @Github: https://github.com/chanchann
 */


#ifndef EVENTLOOP_H
#define EVENTLOOP_H
#include <boost/noncopyable.hpp>
#include <memory>
#include <thread>
#include <iostream>

namespace burger {
namespace net {
    
class EventLoop : boost::noncopyable {
public:
    EventLoop();
    ~EventLoop();

    
private:
    bool looping_; // atomic
    const std::thread::id threadId_;  // 当前对象所属线程ID
}


} // namespace net

} // namespace burge 


#endif // EVENTLOOP_H