#ifndef BOUNDEDBLOCKINGQUEUE_H
#define BOUNDEDBLOCKINGQUEUE_H

#include <boost/noncopyable.hpp>
#include <boost/circular_buffer.hpp>
#include <cassert>
#include <mutex>
#include <condition_variable>

namespace burger {

template<typename T>
class BoundedBlockingQueue : boost::noncopyable {
public:
    explicit BoundedBlockingQueue(int maxSize);
    void put(const T& x);
    void put(T&& x);
    T take();
    bool empty() const;
    bool full() const;
    size_t size() const;
    size_t capacity() const;
private:
    mutable std::mutex mutex_;
    std::condition_variable cvNotEmpty_;
    std::condition_variable cvNotFull_;
    boost::circular_buffer<T> queue_;
};

template<typename T>
BoundedBlockingQueue<T>::BoundedBlockingQueue(int maxSize) : queue_(maxSize)
{}

template<typename T>
void BoundedBlockingQueue<T>::put(const T& x) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cvNotFull_.wait(lock, [this] { return !queue_.full(); });
        assert(!queue_.full());
        queue_.push_back(x);
    }
    cvNotEmpty_.notify_one();
}

template<typename T>
void BoundedBlockingQueue<T>::put(T&& x) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cvNotFull_.wait(lock, [this] { return !queue_.full(); });
        assert(!queue_.full());
        queue_.push_back(std::move(x));
    }
    cvNotEmpty_.notify_one();
}

template<typename T>
T BoundedBlockingQueue<T>::take() {
    std::unique_lock<std::mutex> lock(mutex_);
    cvNotEmpty_.wait(lock, [this] { return !queue_.empty(); });
    assert(!queue_.empty());
    T front(std::move(queue_.front()));
    queue_.pop_front();
    cvNotFull_.notify_one();
    return front;
}

template<typename T>
bool BoundedBlockingQueue<T>::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

template<typename T>
bool BoundedBlockingQueue<T>::full() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.full();
}

template<typename T>
size_t BoundedBlockingQueue<T>::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

template<typename T>
size_t BoundedBlockingQueue<T>::capacity() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.compacity();
}









} // namespace burger



#endif // BOUNDEDBLOCKINGQUEUE_H