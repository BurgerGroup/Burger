#ifndef TIMER_H
#define TIMER_H

#include "Callbacks.h"
#include "burger/base/Timestamp.h"
#include "burger/base/Atomic.h"
#include <boost/noncopyable.hpp>

namespace burger {
namespace net {

// internal class for time event
class Timer : boost::noncopyable {
public:
    // when何时执行任务 
    Timer(TimerCallback timercb, Timestamp when, double interval);
    void run() const { timercb_(); }
    // 重置任务，主要是针对需要重复执行的任务
    void restart(Timestamp now);
    // 获取任务的本次到期时间
    Timestamp getExpiration() const { return expiration_; }
    bool isRepeat() const { return repeat_; }
    int64_t getSeq() const { return seq_; }
    static int64_t getNumCreated() { return s_numCreated_.get(); }
private:
    const TimerCallback timercb_;
    Timestamp expiration_; // 下一次的超时时刻
    const double interval_; // 超时时间间隔，如果是一次性定时器，该值为0
    const bool repeat_;    // 是否重复
    const int64_t seq_;     // 定时器序号
    static AtomicInt64 s_numCreated_;   // 定时器计数，当前已经创建的定时器数量
};
} // namespace net

} // namespace burger


#endif // TIMER_H