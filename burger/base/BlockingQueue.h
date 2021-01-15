#ifndef BLOCKINGQUEUE_H
#define BLOCKINGQUEUE_H

#include <cassert>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <boost/noncopyable.hpp>

namespace burger {

template<typename T>
class BlockingQueue : boost::noncopyable {
public:
    BlockingQueue() = default;
    void put(const T& x);
    void put(T&& x);
    T take();
    size_t size() const;

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::deque<T> queue_;
};

template<typename T>
void BlockingQueue<T>::put(const T& x) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(x);
    }
    cv_.notify_one();
}

template<typename T>
void BlockingQueue<T>::put(T&& x) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(std::move(x));
    }
    cv_.notify_one();
}

template<typename T>
T BlockingQueue<T>::take()  {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !queue_.empty(); });
    assert(!queue_.empty());
    T front(std::move(queue_.front()));  // 此处调用移动构造函数，optim
    queue_.pop_front();
    return front;
}

template<typename T>
size_t BlockingQueue<T>::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
} 

} // namespace burger




#endif // BLOCKINGQUEUE_H