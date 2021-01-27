#include "TimerQueue.h"

namespace burger {
namespace net {
namespace detail {

int createTimerfd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if(timerfd < 0) {
        CRITICAL("Failed in timerfd_create");
    }
    return timerfd;
}

// 现在距离超时还有多久
struct timespec howMuchTimeFromNow(Timestamp when) {
    int64_t microseconds = when.microSecondsSinceEpoch() 
                            - Timestamp::now().microSecondsSinceEpoch();
    if(microseconds < 100) {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

// 处理超时事件，超时后，timerfd变为可读，howmany表示超时的次数
// 将事件读出来，避免陷入Loop忙碌状态
void readTimerfd(int timerfd, Timestamp now) {
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
    TRACE("TimerQueue::handleRead {} at {}", howmany, now.toString());
    if(n != sizeof(howmany)) {
        ERROR("TimerQueue::handleRead reads {} bytes instead of 8", n);
    }   
}

// 重新设置定时器描述符关注的定时事件
void resetTimerfd(int timerfd, Timestamp expiration) {
    // wake up loop by timerfd_settime
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof(newValue));
    bzero(&oldValue, sizeof(oldValue));
    newValue.it_value = howMuchTimeFromNow(expiration);  // 获取与现在的时间差值，然后设置关注事件
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    // timerfd_settime() return 0 on success; on error they return -1, and set errno to indicate the error.
    if(ret) {
        ERROR("timer_settime error");
    }
}

} // namespace detail
} // namespace net
} // namespace burger
