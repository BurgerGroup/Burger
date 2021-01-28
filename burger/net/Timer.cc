#include "Timer.h"

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

void Timer::restart(Timestamp now) {
    if(repeat_) {
        // 如果重复，那么将时间设置为下次过期的时间
        expiration_ = addTime(now, interval_);
    } else {
        expiration_ = Timestamp::invalid();
    }
}



