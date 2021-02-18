#ifndef CHANNEL_H
#define CHANNEL_H

#include "burger/base/Timestamp.h"
#include "burger/base/Log.h"
#include <memory>
#include <boost/noncopyable.hpp>
#include <functional>
#include <sstream>
#include <cassert>
namespace burger {
namespace net {

class EventLoop;
class Epoll;
// This class doesn't own the file descriptor.
// Channel的成员函数都只能再IO线程调用，所以更新数据不需要加锁
class Channel : boost::noncopyable {
public: 
    enum class Status {
        kNew = 0,   // 不在epoll队列里，也不在channelMap_ 中
        kAdded = 1,     // 正在epoll队列当中 
        kDismissed = 2,      // 曾经在epoll队列当中过，但暂时被dissmiss了，仍在channelMap_中
    };
    static std::string statusTostr(Status status);
    
public:
    using EventCallback =  std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);
    // void setReadCallback(const ReadEventCallback& cb) { readCallback_ = cb; }
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }
    
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    // For Epoll
    Status getStatus() const { return status_; }
    void setStatus(Status status) { status_ = status; }
    
    // For debug 
    std::string eventsToString() const;
    std::string reventsToString() const;
    /*Channel通常作为其它类的成员，比如TcpConnection，而Channel的回调函数通常和TcpConnection
    通过std::bind绑定,当Epoll通知该Channel的回调时，Channel会调用TcpConnection对应的回调，
    而此时TcpConnection的生命周期尚未可知，此时tie_保存TcpConnection的this指针，通过将tie_
    的weak_ptr提升为shared_ptr成功与否判断TcpConnection是否健在。*/
    void tie(const std::shared_ptr<void>&);
    int getFd() const { return fd_; }
    uint32_t getEvents() const { return events_; }
    void setRevents(uint32_t revent) { revents_ = revent; }

    void doNotLogHup() { logHup_ = false; }    
    EventLoop* ownerLoop() { return loop_; }
    void remove();

private:
    static std::string eventsToString(int fd, int event);
    void update();
    void handleEventWithGuard(Timestamp receiveTime);
private:
    static const uint32_t kNoneEvent;
    static const uint32_t kReadEvent;
    static const uint32_t kWriteEvent;

    EventLoop* loop_;
    const int fd_;
    uint32_t events_;
    uint32_t revents_;
    Status status_;

    std::weak_ptr<void> tie_;
    bool tied_;
    bool eventHandling_;
    bool addedToEpoll_;  //是否注册到Epoll中监听
    bool logHup_;  
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};

} // namespace net

} // namespace burger


#endif // CHANNEL_H