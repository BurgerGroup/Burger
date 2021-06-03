#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include <functional>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <boost/noncopyable.hpp>
#include <string>

namespace burger {
namespace net {

class EventLoop;
/**
 * @brief 创建一个线程，在线程函数中创建一个EventLoop对象并调用loop
*/
class EventLoopThread : boost::noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
    ~EventLoopThread();
    EventLoop* startLoop();     // 启动线程，该线程就成为了IO线程
private:
    void threadFunc();  

    EventLoop* loop_;
    bool exiting_;
    // https://thispointer.com/c11-how-to-use-stdthread-as-a-member-variable-in-class/
    // We need to take care that objects of this class should also be move only.
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    ThreadInitCallback threadInitcallback_;  // 回调函数再EventLoop::loop之前被调用,默认为空
};
} // namespace net

} // namespace burger



#endif // EVENTLOOPTHREAD_H