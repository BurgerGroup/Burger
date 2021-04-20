#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "burger/base/Coroutine.h"
#include "burger/base/Timestamp.h"
#include "Processor.h"
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <boost/noncopyable.hpp>

namespace burger {
namespace net {

class Processor;
class ProcessThread;
class TimerQueue;
class Scheduler : boost::noncopyable {
public:
    using ptr = std::shared_ptr<Scheduler>;
    Scheduler(size_t threadNum = 1);
    ~Scheduler();

    void start();
    void startAsync();
    void wait();
    void stop();
    void addTask(Coroutine::Callback task, std::string name = "");
    int64_t runAt(Timestamp time, Coroutine::ptr coroutine);
    int64_t runAfter(uint64_t microDelay, Coroutine::ptr coroutine);
    int64_t runEvery(uint64_t microInterval, Coroutine::ptr coroutine);
    void cancel(int64_t);
protected:
    Processor* pickOneProcesser();
private:
    bool running_ = false;
    size_t threadNum_;
    Processor mainProc_;
    std::vector<Processor *> workProc_;  // unique_ptr here?
    std::vector<std::shared_ptr<ProcessThread> > procThreadList_;
    //单独一个线程处理定时任务
    Processor* timerProc_;
    std::shared_ptr<ProcessThread> timerThread_;
    std::unique_ptr<TimerQueue> timerQueue_;
    std::thread thread_;
    std::mutex mutex_;

};
} // namespace net

    
} // namespace burger







#endif // SCHEDULER_H