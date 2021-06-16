#include "CoTimerQueue.h"
#include "Processor.h"
#include "Timer.h"
#include "TimerId.h"

namespace burger{
namespace net {


// todo : 没写好 -- addTimerInProc
// todo : cancelTimerInProc

CoTimerQueue::CoTimerQueue(Processor* proc) 
    : proc_(proc) {
}

TimerId CoTimerQueue::addTimer(Coroutine::ptr co, Timestamp when, double interval) {
    Timer::ptr timer = std::make_shared<Timer>(co, proc_, when, interval);
    bool earliestChanged = false;
    earliestChanged = insert(timer); // 是否将timer插入set首部，比现有队列里的所有的都更早

    if(earliestChanged) {   // 如果插入首部，更新timerfd关注的到期时间
        DEBUG("earliest reset");
        detail::resetTimerfd(timerfd_, timer->getExpiration());   // 启动定时器
    }
    DEBUG("add timer {}", timer->getSeq());
    return TimerId(timer, timer->getSeq());
}

TimerId CoTimerQueue::addTimer(TimerCallback cb, const std::string& name, Timestamp when, double interval) {
    Timer::ptr timer = std::make_shared<Timer>(cb, name, proc_, when, interval);
    bool earliestChanged = false;
    earliestChanged = insert(timer); // 是否将timer插入set首部，比现有队列里的所有的都更早

    if(earliestChanged) {   // 如果插入首部，更新timerfd关注的到期时间
        DEBUG("earliest reset");
        detail::resetTimerfd(timerfd_, timer->getExpiration());   // 启动定时器
    }
    DEBUG("add timer {}", timer->getSeq());
    return TimerId(timer, timer->getSeq());
}

void CoTimerQueue::cancel(TimerId timerId) {
    auto timer = timerId.timer_;
    auto cancelTimer = Entry(timer->getExpiration(), timer);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if(timers_.find(cancelTimer) != timers_.end()) {
            size_t n = timers_.erase(cancelTimer);
            assert(n == 1);
        } else {
            // 不在列表里，说明可能已经到期getExpiered出来了，并且正在调用定时器的回调函数
            cancelingTimers_.insert(timer);
        }
    }
    DEBUG("cancel Timer {}", cancelTimer.second->getSeq()); 
}

bool CoTimerQueue::insert(std::shared_ptr<Timer> timer) {
    bool earliestChanged = false;
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
    return earliestChanged;
}

// todo: 需要重构，非常丑陋的代码，隐藏co，全部接口给cb
void CoTimerQueue::dealWithExpiredTimer() {
    std::vector<Entry> expiredList;
    detail::readTimerfd(timerfd_, Timestamp::now());  
    
    if(proc_->stoped()) return;

    expiredList = getExpiredList(Timestamp::now());

    for(const auto& pair : expiredList) {
        Timer::ptr oldTimer = pair.second;

        if(cancelingTimers_.find(oldTimer) != cancelingTimers_.end()) {
            continue;
        }

        assert(oldTimer->getProcessor() != nullptr);
        
        if(oldTimer->getCo() != nullptr) {
            oldTimer->getProcessor()->addTask(oldTimer->getCo());
        } else {
            oldTimer->getProcessor()->addPendingTask(oldTimer->getCb(), oldTimer->getName());
        }
        if(oldTimer->getInterval() > 0) {
            Timestamp newTs = Timestamp::now() + oldTimer->getInterval();
            oldTimer->setExpiration(newTs);
            if(oldTimer->getCo() != nullptr) {
                oldTimer->setCo(oldTimer->getProcessor()->resetAndGetCo(oldTimer->getCo()->getCallback(), "repeat"));
            } else {
                oldTimer->setCo(std::make_shared<Coroutine>(oldTimer->getCb()));
            }
            timers_.insert(Entry(newTs, oldTimer));
        } 
    }
    cancelingTimers_.clear(); // 优化

    Timestamp ts;
    if(findFirstTimestamp(Timestamp::now(), ts)) {
        detail::resetTimerfd(timerfd_, ts);
    }    
}

}
}