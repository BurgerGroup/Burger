#include "ThreadPool.h"

using namespace burger;

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
        // TODO : 这里还需要深刻理解
    }
    std::for_each(threads_.begin(), threads_.end(), 
                    std::mem_fn(&std::thread::join));
}

size_t Threadpool::queueSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

// 增加一个task进来
void Threadpool::run(Task task) {
    //如果线程池为空，说明线程池未分配线程. 由当前线程执行
    if(threads_.empty()) task();  
    else {
        std::unique_lock<std::mutex> lock(mutex_);
        cvNotFull_.wait(lock, [this] { return (!isFull() || !running_); });
        if(!running_) return;
        assert(!isFull());
        queue_.push_back(std::move(task));  // TODO : WHY MOVE , WHY NOT emplace_bacl
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




