#ifndef TIMER_H
#define TIMER_H

#include "Callbacks.h"
#include "burger/base/Timestamp.h"
#include "burger/base/Atomic.h"
#include "burger/base/Coroutine.h"
#include <boost/noncopyable.hpp>
#include <string>

namespace burger {
namespace net {
class Processor;
// internal class for time event
// timerfd_create, setime都没操作，只是高层次抽象
class Timer : boost::noncopyable {
public:
    using ptr = std::shared_ptr<Timer>;
    // when何时执行任务 
    Timer(TimerCallback timercb, Timestamp when, double interval);
    // for co
    Timer(std::shared_ptr<Coroutine> co, Processor* proc, Timestamp when, double interval);
    Timer(TimerCallback timercb, const std::string& name, Processor* proc, Timestamp when, double interval);

    void run() const { timercb_(); } 
    // 重置任务，主要是针对需要重复执行的任务
    void restart(Timestamp now);
    std::shared_ptr<Coroutine> getCo() { return co_; }
    TimerCallback getCb() const {return timercb_; }
    const std::string& getName() const { return name_; }
    void setCo(std::shared_ptr<Coroutine> co) { co_ = co; }
    Processor* getProcessor() { return proc_; }

    // 获取任务的本次到期时间
    Timestamp getExpiration() const { return expiration_; }
    void setExpiration(Timestamp expiration) { expiration_ = expiration; }
    double getInterval() { return interval_; }
    bool isRepeat() const { return repeat_; }
    uint64_t getSeq() const { return seq_; }
    static uint64_t getNumCreated() { return s_numCreated_.get(); }
private:
    const TimerCallback timercb_;
    std::string name_;
    std::shared_ptr<Coroutine> co_;
    Processor* proc_;
    Timestamp expiration_; // 下一次的超时时刻
    const double interval_; // 超时时间间隔，如果是一次性定时器，该值为0
    const bool repeat_;    // 是否重复
    const uint64_t seq_;     // 定时器序号
    static AtomicInt64 s_numCreated_;   // 定时器计数，当前已经创建的定时器数量
};
} // namespace net

} // namespace burger


#endif // TIMER_H