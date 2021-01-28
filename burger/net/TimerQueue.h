#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include "burger/base/Timestamp.h"
#include <boost/noncopyable.hpp>
#include <memory>
#include <set>
#include <vector>
#include <functional>
#include "EventLoop.h"
#include <sys/timerfd.h>
#include "Callbacks.h"

namespace burger {
namespace net {
class TimerId;
class Timer;
class EventLoop;
class Channel;
// A best effort timer queue
// No guarantee that the callback will be on time
class TimerQueue : boost::noncopyable {
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();
    // 一定线程安全，可以跨线程调用，通常情况下被其他线程调用
    TimerId addTimer(TimerCallback timercb, Timestamp when, double interval);
    void cancel(TimerId timerId);
private:
    // KEY POINT 
    // set是排序的！！！
    using Entry = std::pair<Timestamp, std::shared_ptr<Timer> >;
    using TimerSet = std::set<Entry>;
    using ActiveTimer = std::pair<std::shared_ptr<Timer>, int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;

    // 以下函数只可能在所属的IO线程中调用，因而不用加锁
    // 服务器性能杀手之一就是锁竞争，所以尽量少用
    void addTimerInLoop(std::shared_ptr<Timer> timer);
    void cancelInLoop(TimerId timerId);
    void handleRead();
    // 返回超时的定时器列表
    std::vector<Entry> getExpiredList(Timestamp now);
    // 处理完任务后，需要根据是否重复，将某些任务重新放入
    void reset(const std::vector<Entry>& expiredList, Timestamp now);
    // 在两个set中插入定时器
    bool insert(std::shared_ptr<Timer> timer);
private:
    EventLoop* loop_;
    const int timerfd_; 
    std::unique_ptr<Channel> timerfdChannel_;
    // timers_和activetimers_保存的是相同的数据
    // timers_按照到期时间排列，activeTimers_按照对象地址排序
    TimerSet timers_;
    ActiveTimerSet activeTimers_;    // TODO : 这个可以不用吗
    ActiveTimerSet cancelingTimers_;    // 保存的是取消的定时器
    bool callingExpiredTimers_;     // atomic, 是否正在处理超时事件

};

} // namespace net
} // namespace burger

#endif // TIMERQUEUE_H