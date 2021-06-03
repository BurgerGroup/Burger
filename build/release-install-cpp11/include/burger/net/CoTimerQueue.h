#ifndef COTIMERQUEUE_H
#define COTIMERQUEUE_H

#include "burger/net/ITimerQueue.h"
#include "burger/base/Coroutine.h"

namespace burger {
namespace net {
    
class Processor;
class Scheduler;

class CoTimerQueue : public ITimerQueue {
friend class Processor;
public:
    CoTimerQueue(Processor* proc);
    ~CoTimerQueue() = default;

    // 封装成runAt,runAfter等使用
    TimerId addTimer(Coroutine::ptr co, Timestamp when, double interval = 0);
    TimerId addTimer(TimerCallback cb, const std::string& name, Timestamp when, double interval = 0);
    void cancel(TimerId timerId);
private:
    // 插入定时器
    bool insert(std::shared_ptr<Timer> timer);
    void dealWithExpiredTimer(); 
private:
    Processor* proc_;
    // const int timerfd_; 
    // TimerSet timers_;
    // std::set<std::shared_ptr<Timer> > cancelingTimers_;    // 保存的是取消的定时器 --- todo 此处数据结构可否优化
    // bool callingExpiredTimers_;     // atomic, 是否正在处理超时事件
    // std::mutex mutex_;
};

} // namespace net
} // namespace burger


#endif // COTIMERQUEUE_H