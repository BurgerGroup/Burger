#ifndef PROCESSERTHREAD_H
#define PROCESSERTHREAD_H

#include <boost/noncopyable.hpp>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace burger {
namespace net {

class Scheduler;
class Processor;

// todo : EventLoopThread
class ProcessThread :public boost::noncopyable {
public:
    using ptr = std::shared_ptr<ProcessThread>;

    ProcessThread(Scheduler* scheduler);
    ~ProcessThread();

    Processor* startProcess();
    void join();
    
private:
    void threadFunc();
    std::thread thread_;	
    Scheduler* scheduler_;
    Processor* proc_ = nullptr;
    std::mutex mutex_;
    std::condition_variable cv_;
};
    
} // namespace net

} // namespace burger




#endif // PROCESSERTHREAD_H
