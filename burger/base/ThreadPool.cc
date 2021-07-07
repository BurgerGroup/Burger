#include "ThreadPool.h"
#include "Log.h"

using namespace burger;

// why not RAII ? Why not automatically start 
Threadpool::Threadpool(const std::string& name) 
        : name_(name), maxQueueSize_(0), running_(false)
{}

Threadpool::~Threadpool() {
    //如果线程池在运行，那就要进行内存处理，在stop()函数中执行
    if(running_) stop(); 
}

void Threadpool::start(int numThreads)  {
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(numThreads);
    for(int i = 0; i < numThreads; i++) {
        threads_.emplace_back(std::thread(&Threadpool::runInThread, this));
    }
    if(numThreads == 0 && threadInitCallback_) {
        threadInitCallback_();
    }
}

void Threadpool::stop() {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        running_ = false;
        cvNotEmpty_.notify_all();
        cvNotFull_.notify_all();
        // 包工头唤醒把所有活干完
        // 然后其他的生产者消费者再互相唤醒
    }
    std::for_each(threads_.begin(), threads_.end(), 
                    std::mem_fn(&std::thread::join));
}

size_t Threadpool::queueSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

// todo : meaning ?
// There is no move-only version of std::function in C++ as of C++14.
// So we don't need to overload a const& and an && versions
// as we do in (Bounded)BlockingQueue.
// https://stackoverflow.com/a/25408989
// void Threadpool::run(Task task) {
//     //如果线程池为空，说明线程池未分配线程. 由当前线程执行
//     if(threads_.empty()) task();  
//     else {
//         std::unique_lock<std::mutex> lock(mutex_);
//         cvNotFull_.wait(lock, [this] { return (!isFull() || !running_); });
//         if(!running_) return;
//         assert(!isFull());
//         queue_.push_back(std::move(task));  
//         cvNotEmpty_.notify_one();
//     }
// }


// 增加一个task进来
// 生产者
void Threadpool::run(const Task& task) {
    // 如果线程池为空，说明线程池未分配线程. 由当前线程执行
    // INFO("const reference");
    if(threads_.empty()) task();  
    else {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cvNotFull_.wait(lock, [this] { return (!isFull() || !running_); });
            if(!running_) return;
            assert(!isFull());
            queue_.emplace_back(task); 
        }
        cvNotEmpty_.notify_one();
    }
}
void Threadpool::run(Task&& task) {
    // INFO("right value");
    if(threads_.empty()) task();  
    else {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cvNotFull_.wait(lock, [this] { return (!isFull() || !running_); });
            if(!running_) return;
            assert(!isFull());
            queue_.emplace_back(std::move(task));  
        }
        cvNotEmpty_.notify_one();
    } 
}

bool Threadpool::isFull() const {
    // assert(mutex_.is_locked());
    // https://stackoverflow.com/questions/21892934/how-to-assert-if-a-stdmutex-is-locked
    return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

void Threadpool::runInThread() {
    try {
        if (threadInitCallback_) {
            threadInitCallback_();
        }
        while (running_) {
            Task task(take());
            if (task) {
                task();
            }
        }
    } catch (const Exception& ex) {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
        abort();
    } catch (const std::exception& ex) {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    } catch (...) {
        fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
        throw; // rethrow
    }
}

Threadpool::Task Threadpool::take() {
    std::unique_lock<std::mutex> lock(mutex_);
    cvNotEmpty_.wait(lock, [this] { return (!queue_.empty() || !running_); });
    Task task;
    if(!queue_.empty()) {
        task = queue_.front();
        queue_.pop_front();
        if(maxQueueSize_ > 0) 
            cvNotFull_.notify_one();
    }
    return task;
}






