#include "TimerQueue.h"
#include "Timer.h"
#include "TimerId.h"
#include "Channel.h"
#include "EventLoop.h"

using namespace burger;
using namespace burger::net;


TimerQueue::TimerQueue(EventLoop* loop):
    loop_(loop),
    timerfdChannel_(util::make_unique<Channel>(loop, timerfd_)),
    callingExpiredTimers_(false) {
    // 设置Channel的常规步骤
    timerfdChannel_->setReadCallback(std::bind(&TimerQueue::handleRead, this));
    // we are always reading the timerfd, we disarm it with timerfd_settime.
    timerfdChannel_->enableReading();
}

TimerQueue::~TimerQueue() {
    timerfdChannel_->disableAll();  // channel不再关注任何事件
    timerfdChannel_->remove();       // 在三角循环中删除此channel
}

// 添加了Timer返回一个外部类timerId供外部使用
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
    bool earliestChanged = insert(timer);  // 是否将timer插入set首部，比现有队列里的所有的都更早
    if(earliestChanged) {   // 如果插入首部，更新timerfd关注的到期时间
        detail::resetTimerfd(timerfd_, timer->getExpiration());   // 启动定时器
    }
}

void TimerQueue::cancelInLoop(TimerId timerId) {
    loop_->assertInLoopThread();
    auto timer = timerId.timer_;
    auto cancelTimer = Entry(timer->getExpiration(), timer);
    if(timers_.find(cancelTimer) != timers_.end()) {
        size_t n = timers_.erase(cancelTimer);
        assert(n == 1);
    } else {
        // 不在列表里，说明可能已经到期getExpiered出来了，并且正在调用定时器的回调函数
        cancelingTimers_.insert(timer);
    }
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    detail::readTimerfd(timerfd_, now);   // 防止一直出现可读事件，造成loop忙碌
    auto expiredList = getExpiredList(now);  // 获取此时刻之前所有定时器列表
    callingExpiredTimers_ = true;
    // 清空去存已经从timers_取出过期的正在执行的timer,在reset的时候这些timer里面已取消的定时器就不必再重启了
    cancelingTimers_.clear();   // todo : 理
    for(const Entry& expired: expiredList) {
        expired.second->run();
    }
    callingExpiredTimers_ = false;
    // 如果不是一次性定时器，那么需要重启
    reset(expiredList, now);
}

bool TimerQueue::insert(std::shared_ptr<Timer> timer) {
    bool earliestChanged = false; 
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
    return earliestChanged;
}


