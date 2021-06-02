#ifndef SCHEDULER_H
#define SCHEDULER_H


#include "burger/base/Timestamp.h"
#include "Processor.h"
#include "burger/base/Singleton.h"
#include "burger/base/Coroutine.h"
#include "Callbacks.h"
#include "TimerId.h"
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <condition_variable>
#include <boost/noncopyable.hpp>

namespace burger {
namespace net {

class Processor;
class ProcessThread;

class Scheduler : boost::noncopyable {
public:
    using ptr = std::shared_ptr<Scheduler>;
    Scheduler();
    ~Scheduler();

    void setWorkProcNum(size_t workProcNum);
    void startAsync();
    void wait();
    void stop();
    
    void addTask(const Coroutine::Callback& task, const std::string& name = "");
    void addMainTask(const Coroutine::Callback& task, const std::string& name = "");
    // distribute Task to all work Processor
    void distributeTask(const Coroutine::Callback& task, const std::string& name = "");
    // todo: 这里是没有main Processor的
    // 定时完成协程任务（这里的协程一定是processor中已经存在的，而不是在其他地方新建的）
    TimerId runAt(Timestamp when, Coroutine::ptr co); 
    TimerId runAfter(double delay, Coroutine::ptr co);
    TimerId runEvery(double interval, Coroutine::ptr co);
    TimerId runAt(Timestamp when, TimerCallback cb, const std::string& name = "");
    TimerId runAfter(double delay, TimerCallback cb, const std::string& name = "");
    TimerId runEvery(double interval, TimerCallback cb, const std::string& name = "");
    void cancel(TimerId timerId);

    Processor* getMainProcessor() { return mainProc_; }
    std::vector<Processor *> getWorkProcList() { return workProcVec_; }
    size_t getWorkProcNum();
    Processor* pickOneWorkProcessor();

private:
    void start();
    void joinThread();
private:
    bool running_ = false;
    bool quit_ = false;
    size_t workProcNum_ = 0;
    Processor* mainProc_;
    std::vector<Processor *> workProcVec_;  // todo : 优先队列
    std::vector<std::unique_ptr<ProcessThread> > workThreadVec_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::condition_variable quitCv_;
};

} // namespace net    
} // namespace burger

#endif // SCHEDULER_H