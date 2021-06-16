#include "ITimerQueue.h"
#include "Timer.h"
#include "TimerId.h"

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
    uint64_t microseconds = when.microSecondsSinceEpoch() 
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
    newValue.it_value = detail::howMuchTimeFromNow(expiration);  // 获取与现在的时间差值，然后设置关注事件
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    // timerfd_settime() return 0 on success; on error they return -1, and set errno to indicate the error.
    if(ret) {
        ERROR("timer_settime error");
    }
}
} // namespace detail



ITimerQueue::ITimerQueue()
    : timerfd_(detail::createTimerfd())
    {}
    
ITimerQueue::~ITimerQueue() {
    ::close(timerfd_);
}


// 获取并在timers_里删除过时的
std::vector<ITimerQueue::Entry> ITimerQueue::getExpiredList(Timestamp now) {
    std::vector<Entry> expiredList;
    Entry midEntry(now, std::shared_ptr<Timer>());
    // 以midEntry 为上界，取出所有时间戳小于等于now的entry
    // 此时begin和mid之间，就是过期的任务, timerSet是按照时间戳升序排列
    auto mid = timers_.lower_bound(midEntry);
    // 要么任务都过期，要么now小于mid，mid不属于要过期的那个
    assert(mid == timers_.end() || now < mid->first);
    std::copy(timers_.begin(), mid, std::back_inserter(expiredList));
    timers_.erase(timers_.begin(), mid);
    // ! 此处有rvo优化
    return expiredList;
}

void ITimerQueue::reset(const std::vector<Entry>& expiredList, Timestamp now) {
    for(const Entry& expired : expiredList) {
        auto timer = expired.second;
        // 如果是重复的定时器并且是未取消的定时器，则重启该定时器
        if(expired.second->isRepeat() && 
            cancelingTimers_.find(timer) == cancelingTimers_.end()) {
            timer->restart(now);
            insert(timer);
        }
    } 
    Timestamp nextExpire;
    if(!timers_.empty()) {
        nextExpire = timers_.begin()->second->getExpiration();
    }   
    if(nextExpire.valid()) {
        // 如果到期时间不为0，重新设置timerfd应该关注的时间
        detail::resetTimerfd(timerfd_, nextExpire);
    }
}

bool ITimerQueue::findFirstTimestamp(const Timestamp& now, Timestamp& ts) {
    // every processor / eventloop get own timerQueue
    // so we don't need lock here
    // std::lock_guard<std::mutex> lock(mutex_);  
    if(timers_.empty()) return false;
    ts = timers_.begin()->first;
    return true;
}

} // net
} // burger