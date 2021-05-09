#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include "burger/net/ITimerQueue.h"

namespace burger {
namespace net {
    
class TimerId;
class Timer;
class EventLoop;
class Channel;

// A best effort timer queue
// No guarantee that the callback will be on time
class TimerQueue : public ITimerQueue {
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();
    // 一定线程安全，可以跨线程调用，通常情况下被其他线程调用
    // 供EventLoop使用，封装成runAt,runAfter等使用
    TimerId addTimer(TimerCallback timercb, Timestamp when, double interval);
    void cancel(TimerId timerId);
private:
    // set是排序的, 这样可以根据当前时间快速查找添加删除 Timer，并且能够处理相同的时间key的问题
    // using Entry = std::pair<Timestamp, std::shared_ptr<Timer> >;
    // using TimerSet = std::set<Entry>;
    // 以下函数只可能在所属的IO线程中调用，因而不用加锁
    // 服务器性能杀手之一就是锁竞争，所以尽量少用
    void addTimerInLoop(std::shared_ptr<Timer> timer);
    void cancelInLoop(TimerId timerId);
    void handleRead();
    // 继承 : 返回超时的定时器列表, 并从timers_中删除
    // std::vector<Entry> getExpiredList(Timestamp now);
    // 继承 : 处理完任务后，需要根据是否重复，将某些任务重新放入
    // void reset(const std::vector<Entry>& expiredList, Timestamp now);
    // 插入定时器
    bool insert(std::shared_ptr<Timer> timer);

private:
    EventLoop* loop_;
    // const int timerfd_; 
    std::unique_ptr<Channel> timerfdChannel_;
    // TimerSet timers_;
    // std::set<std::shared_ptr<Timer> > cancelingTimers_;    // 保存的是取消的定时器 --- todo 此处数据结构可否优化
    bool callingExpiredTimers_;     // atomic, 是否正在处理超时事件
    // std::mutex mutex_;
};

} // namespace net
} // namespace burger

#endif // TIMERQUEUE_H