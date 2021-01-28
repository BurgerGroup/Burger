#include "TimerQueue.h"
#include "Timer.h"
#include "TimerId.h"
#include "Channel.h"
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

using namespace burger;
using namespace burger::net;
using namespace burger::net::detail;

TimerQueue::TimerQueue(EventLoop* loop):
    loop_(loop),
    timerfd_(createTimerfd()),
    timerfdChannel_(util::make_unique<Channel>(loop, timerfd_)),  // 这里可以直接传参构造
    callingExpiredTimers_(false) {
    timerfdChannel_->setReadCallback(std::bind(&TimerQueue::handleRead, this));
    // we are always reading the timerfd, we disarm it with timerfd_settime.
    // 设置Channel的常规步骤
    timerfdChannel_->enableReading();
}

TimerQueue::~TimerQueue() {
    timerfdChannel_->disableAll();  // channel不再关注任何事件
    timerfdChannel_->remove();       // 在三角循环中删除此channel
    ::close(timerfd_);
}

TimerId TimerQueue::addTimer(TimerCallback timercb, Timestamp when, double interval) {
    auto timer = std::make_shared<Timer>(std::move(timercb), when, interval);
    // 跨线程调用
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->getSeq());
}

void TimerQueue::cancel(TimerId timerId) {
    loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
}



void TimerQueue::addTimerInLoop(std::shared_ptr<Timer> timer) {
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);  // 是否将timer插入set首部
    if(earliestChanged) {   // 如果插入首部，更新timerfd关注的到期时间
        resetTimerfd(timerfd_, timer->getExpiration());   // 启动定时器
    }
}

void TimerQueue::cancelInLoop(TimerId timerId) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    // 要cancel的实体 -- activeSet的索引
    ActiveTimer timer(timerId.timer_, timerId.seq_);
    auto it = activeTimers_.find(timer);
    if(it != activeTimers_.end()) {
        size_t n = timers_.erase(Entry(it->first->getExpiration(), it->first));
        assert(n == 1);
        activeTimers_.erase(it);
    } else if(callingExpiredTimers_) {
        // TODO : 这里不太清楚
        cancelingTimers_.insert(timer);
    }
    assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);   // 防止一直出现可读事件，造成loop忙碌
    auto expiredList = getExpiredList(now);
    callingExpiredTimers_ = true;
    // TODO : 更新完成后马上就是重置，重置时依赖已经取消的定时器的条件，所以这里先清空
    cancelingTimers_.clear();  
    for(const Entry& expired: expiredList) {
        expired.second->run();
    }
    callingExpiredTimers_ = false;
    reset(expiredList, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpiredList(Timestamp now) {
    assert(timers_.size() == activeTimers_.size());
    std::vector<Entry> expiredList;
    Entry midEntry(now, std::shared_ptr<Timer>());
    // 以midEntry 为上界，取出所有时间戳小于now的entry
    // 此时begin和mid之间，就是过期的任务, timerSet是按照时间戳升序排列
    auto mid = timers_.lower_bound(midEntry);
    // 要么任务都过期，要么now小于mid，mid不属于要过期的那个
    assert(mid == timers_.end() || now < mid->first);
    std::copy(timers_.begin(), mid, std::back_inserter(expiredList));
    timers_.erase(timers_.begin(), mid);
    // 从ActiveTimers_里面同步删除
    for(const Entry& expired : expiredList) {
        ActiveTimer timer(expired.second, expired.second->getSeq());
        size_t n = activeTimers_.erase(timer);
        assert(n == 1);
    }
    assert(timers_.size() == activeTimers_.size());
    return expiredList;
}

void TimerQueue::reset(const std::vector<Entry>& expiredList, Timestamp now) {
    Timestamp nextExpire;
    for(const Entry& expired : expiredList) {
        ActiveTimer timer(expired.second, expired.second->getSeq());
        if(expired.second->isRepeat() && 
            cancelingTimers_.find(timer) == cancelingTimers_.end()) {
            expired.second->restart(now);
            insert(expired.second);
        }
    } 
    if(!timers_.empty()) {
        nextExpire = timers_.begin()->second->getExpiration();
    }   
    if(nextExpire.valid()) {
        // 如果到期时间不为0，重新设置timerfd应该关注的时间
        resetTimerfd(timerfd_, nextExpire);
    }
}

bool TimerQueue::insert(std::shared_ptr<Timer> timer) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size()); // 判断两个set是否同步
    bool earliestChanged = false;
    Timestamp when = timer->getExpiration();
    auto it = timers_.begin();
    if(it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }
    {
        // 注意insert的返回参数
        auto res = timers_.insert(Entry(when, timer));
        assert(res.second);
    }
    {
        auto res = activeTimers_.insert(ActiveTimer(timer, timer->getSeq()));
        assert(res.second);
    }
    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}

