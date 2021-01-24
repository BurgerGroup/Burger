#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "Exception.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <boost/noncopyable.hpp>
#include <cassert>
#include <vector>
#include <deque>
#include <string>
#include <algorithm>
#include <boost/thread/is_locked_by_this_thread.hpp>
// TODO : 是否需要改成变长线程池

namespace burger {

class Threadpool : boost::noncopyable {
public:
    using Task = std::function<void()>;
    explicit Threadpool(const std::string& name = std::string("ThreadPool"));
    ~Threadpool();
    // Must be called before start().
    void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
    // There is no move-only version of std::function in C++ as of C++14.   
    void setThreadInitCallback(const Task& cb) { threadInitCallback_ = cb; }
    // void setThreadInitCallback(const Task&& cb) { threadInitCallback_ = cb; }
    void start(int numThreads);
    void stop();
    const std::string& name() const { return name_; }
    size_t queueSize() const;
    void run(Task task);

private:
    bool isFull() const;  // require lock
    void runInThread();
    Task take();

private:
    mutable std::mutex mutex_;
    std::condition_variable cvNotEmpty_;
    std::condition_variable cvNotFull_;
    std::string name_;
    Task threadInitCallback_;
    std::vector<std::thread> threads_;
    std::deque<Task> queue_;
    size_t maxQueueSize_;
    bool running_;
};

    
} // namespace burger



#endif // THREADPOOL_H