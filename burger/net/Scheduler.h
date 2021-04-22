#ifndef SCHEDULER_H
#define SCHEDULER_H


#include "burger/base/Timestamp.h"
#include "Processor.h"
#include "burger/base/Singleton.h"
#include "burger/base/Coroutine.h"
#include "TimerId.h"
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
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
    // void wait();
    void stop();
    
    void addTask(const Coroutine::Callback& task, std::string name = "");
    TimerId runAt(Coroutine::ptr co, Timestamp when);
    TimerId runAfter(Coroutine::ptr co, double delay);
    TimerId runEvery(Coroutine::ptr co, double interval);
    void cancel(TimerId timerId);
protected:
    Processor* pickOneProcesser();
private:
    void joinThread();
private:
    bool running_ = false;
    // bool quit_ = false;
    size_t threadNum_;
    Processor mainProc_;
    std::vector<Processor *> workProcVec_;  // unique_ptr here?
    std::vector<std::shared_ptr<ProcessThread> > workThreadVec_;
    //单独一个线程处理定时任务
    Processor* timerProc_;
    std::shared_ptr<ProcessThread> timerThread_;
    std::unique_ptr<TimerQueue> timerQueue_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    // std::condition_variable quitCv_;

    std::thread joinThrd_;  // for stop

};


} // namespace net

    
} // namespace burger







#endif // SCHEDULER_H