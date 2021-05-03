#include "Timer.h"
#include "burger/base/Coroutine.h"

using namespace burger;
using namespace burger::net;

AtomicInt64 Timer::s_numCreated_;  // be careful static

Timer::Timer(TimerCallback timercb, Timestamp when, double interval):
    timercb_(timercb),
    expiration_(when),
    interval_(interval),
    repeat_(interval_ > 0.0), // 根据interval确定需不需要重复 
    seq_(s_numCreated_.incrementAndGet()) {  // 先加再get，第一个为1
}

Timer::Timer(std::shared_ptr<Coroutine> co, Processor* proc, Timestamp when, double interval)
    : co_(co),
    proc_(proc),
    expiration_(when),
    interval_(interval),
    repeat_(interval_ > 0.0),
    seq_(s_numCreated_.incrementAndGet()) {
}

Timer::Timer(TimerCallback timercb, const std::string& name, Processor* proc, Timestamp when, double interval)
    : timercb_(timercb),
    name_(name),
    co_(nullptr),
    proc_(proc),
    expiration_(when),
    interval_(interval),
    repeat_(interval_ > 0.0),
    seq_(s_numCreated_.incrementAndGet()) {
}

void Timer::restart(Timestamp now) {
    if(repeat_) {
        // 如果重复，那么将时间设置为下次过期的时间
        expiration_ = addTime(now, interval_);
    } else {
        expiration_ = Timestamp::invalid();
    }
}



