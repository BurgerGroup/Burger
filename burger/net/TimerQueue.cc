#include "TimerQueue.h"
#include "processor.h"

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

// for co
TimerQueue::TimerQueue()
    : timerfd_(createTimerfd()),
    mode_(false) {
}

TimerQueue::TimerQueue(EventLoop* loop):
    loop_(loop),
    timerfd_(createTimerfd()),
    timerfdChannel_(util::make_unique<Channel>(loop, timerfd_)),
    callingExpiredTimers_(false),
    mode_(true) {
    // 设置Channel的常规步骤
    timerfdChannel_->setReadCallback(std::bind(&TimerQueue::handleRead, this));
    // we are always reading the timerfd, we disarm it with timerfd_settime.
    timerfdChannel_->enableReading();
}

TimerQueue::~TimerQueue() {
    if(mode_) {
        timerfdChannel_->disableAll();  // channel不再关注任何事件
        timerfdChannel_->remove();       // 在三角循环中删除此channel
    }
    ::close(timerfd_);
}

// 添加了Timer返回一个外部类timerId供外部使用
TimerId TimerQueue::addTimer(TimerCallback timercb, Timestamp when, double interval) {
    auto timer = std::make_shared<Timer>(std::move(timercb), when, interval);
    // 跨线程调用
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->getSeq());
}

TimerId TimerQueue::addTimer(Coroutine::ptr co, Processor* proc, Timestamp when, double interval) {
    Timer::ptr timer = std::make_shared<Timer>(co, proc, when, interval);
    bool earliestChanged = false;
    earliestChanged = insert(timer); // 是否将timer插入set首部，比现有队列里的所有的都更早

    if(earliestChanged) {   // 如果插入首部，更新timerfd关注的到期时间
        DEBUG("earliest reset");
        resetTimerfd(timerfd_, timer->getExpiration());   // 启动定时器
    }
    DEBUG("add timer {}", timer->getSeq());
    return TimerId(timer, timer->getSeq());
}

void TimerQueue::cancel(TimerId timerId) {
    if(mode_) {
        loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
    } else {
        auto timer = timerId.timer_;
        auto cancelTimer = Entry(timer->getExpiration(), timer);
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if(timers_.find(cancelTimer) != timers_.end()) {
                size_t n = timers_.erase(cancelTimer);
                assert(n == 1);
            } else if(callingExpiredTimers_) {
                // 不在列表里，说明可能已经到期getExpiered出来了，并且正在调用定时器的回调函数
                cancelingTimers_.insert(timer);
            }
        }
        DEBUG("cancel Timer {}", cancelTimer.second->getSeq());
    }
}



void TimerQueue::addTimerInLoop(std::shared_ptr<Timer> timer) {
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);  // 是否将timer插入set首部，比现有队列里的所有的都更早
    if(earliestChanged) {   // 如果插入首部，更新timerfd关注的到期时间
        resetTimerfd(timerfd_, timer->getExpiration());   // 启动定时器
    }
}

void TimerQueue::cancelInLoop(TimerId timerId) {
    loop_->assertInLoopThread();
    auto timer = timerId.timer_;
    auto cancelTimer = Entry(timer->getExpiration(), timer);
    if(timers_.find(cancelTimer) != timers_.end()) {
        size_t n = timers_.erase(cancelTimer);
        assert(n == 1);
    } else if(callingExpiredTimers_) {
        // 不在列表里，说明可能已经到期getExpiered出来了，并且正在调用定时器的回调函数
        cancelingTimers_.insert(timer);
    }
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);   // 防止一直出现可读事件，造成loop忙碌
    auto expiredList = getExpiredList(now);  // 获取此时刻之前所有定时器列表
    callingExpiredTimers_ = true;
    // 清空去存已经从timers_取出过期的正在执行的timer,在reset的时候这些timer里面已取消的定时器就不必再重启了
    cancelingTimers_.clear();  
    for(const Entry& expired: expiredList) {
        expired.second->run();
    }
    callingExpiredTimers_ = false;
    // 如果不是一次性定时器，那么需要重启
    reset(expiredList, now);
}

// 获取并在timers_里删除过时的
std::vector<TimerQueue::Entry> TimerQueue::getExpiredList(Timestamp now) {
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

void TimerQueue::reset(const std::vector<Entry>& expiredList, Timestamp now) {
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
        resetTimerfd(timerfd_, nextExpire);
    }
}

bool TimerQueue::insert(std::shared_ptr<Timer> timer) {
    bool earliestChanged = false;
    if(mode_) {  
        loop_->assertInLoopThread();
        Timestamp when = timer->getExpiration();
        auto it = timers_.begin();
        // 如果timers_为空或者when小于timers_中最早到期时间
        if(it == timers_.end() || when < it->first) {
            earliestChanged = true;
        }
        // 注意insert的返回参数， std::pair<iterator,bool>
        auto res = timers_.insert(Entry(when, timer));
        assert(res.second);
    } else {
        Timestamp when = timer->getExpiration();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = timers_.begin();
            // 如果timers_为空或者when小于timers_中最早到期时间
            if(it == timers_.end() || when < it->first) {
                earliestChanged = true;
            }
            // 注意insert的返回参数， std::pair<iterator,bool>
            auto res = timers_.insert(Entry(when, timer));
        }
    }
    return earliestChanged;
}

bool TimerQueue::findFirstTimestamp(const Timestamp& now, Timestamp& ts) {
    std::lock_guard<std::mutex> lock(mutex_);  // todo : need lock here?
    if(timers_.empty()) return false;
    // ts = (*timers_.begin()).first;
    ts = timers_.begin()->first;
    return true;
}

void TimerQueue::dealWithExpiredTimer() {
    std::vector<Entry> expiredList;
    readTimerfd(timerfd_, Timestamp::now());  
    {
        std::lock_guard<std::mutex> lock(mutex_);
        expiredList = getExpiredList(Timestamp::now());
    }
    for(const auto& pair : expiredList) {
        Timer::ptr oldTimer = pair.second;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if(cancelingTimers_.find(oldTimer) != cancelingTimers_.end()) {
                continue;
            }
        }
        assert(oldTimer->getProcessor() != nullptr);
        oldTimer->getProcessor()->addTask(oldTimer->getCo(), "timer");
        if(oldTimer->getInterval() > 0) {
            Timestamp newTs = Timestamp::now() + oldTimer->getInterval();
            oldTimer->setExpiration(newTs);
            // oldTimer->setCo(std::make_shared<Coroutine>(oldTimer->getCo()->getCallback()));
            {
                std::lock_guard<std::mutex> lock(mutex_);
                timers_.insert(Entry(newTs, oldTimer));
            }
        } 
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        cancelingTimers_.clear(); // 优化
    }
    Timestamp ts;
    if(findFirstTimestamp(Timestamp::now(), ts)) {
        resetTimerfd(timerfd_, ts);
    }    
}

