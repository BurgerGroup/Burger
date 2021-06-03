#ifndef ITIMERQUEUE_H
#define ITIMERQUEUE_H

#include "burger/base/Timestamp.h"
#include "burger/base/Log.h"
#include <boost/noncopyable.hpp>
#include <memory>
#include <set>
#include <vector>
#include <mutex>
#include <functional>
#include <sys/timerfd.h>
#include "Callbacks.h"

namespace burger {
namespace net {
namespace detail {

int createTimerfd();
struct timespec howMuchTimeFromNow(Timestamp when);
void readTimerfd(int timerfd, Timestamp now);
void resetTimerfd(int timerfd, Timestamp expiration);

}
    
class TimerId;
class Timer;

// A best effort timer queue
// No guarantee that the callback will be on time
class ITimerQueue : boost::noncopyable {
public:
    ITimerQueue();  // createTimerfd()
    virtual ~ITimerQueue(); // close(timerfd_)

    // TimerId ITimerQueue::addTimer(...)  // 参数不同，该怎么写虚函数？模版？
    virtual void cancel(TimerId timerId) = 0;

protected:
    // set是排序的, 这样可以根据当前时间快速查找添加删除 Timer，并且能够处理相同的时间key的问题
    using Entry = std::pair<Timestamp, std::shared_ptr<Timer>>;
    using TimerSet = std::set<Entry>;
   
    // 返回超时的定时器列表, 并从timers_中删除
    std::vector<Entry> getExpiredList(Timestamp now);
    // 处理完任务后，需要根据是否重复，将某些任务重新放入
    void reset(const std::vector<Entry>& expiredList, Timestamp now);
    bool findFirstTimestamp(const Timestamp& now, Timestamp& ts);

     // 插入定时器
    virtual bool insert(std::shared_ptr<Timer> timer) = 0;

protected:
    const int timerfd_; 
    TimerSet timers_;
    std::set<std::shared_ptr<Timer>> cancelingTimers_;    // 保存的是取消的定时器 --- todo 此处数据结构可否优化
    std::mutex mutex_;
};

} // namespace net
} // namespace burger

#endif // ITIMERQUEUE_H