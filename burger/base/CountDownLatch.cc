#include "CountDownLatch.h"

using namespace burger;

CountDownLatch::CountDownLatch(int count) : count_(count)
{}

void CountDownLatch::wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return count_ <= 0; });
}

void CountDownLatch::countDown() {
    std::lock_guard<std::mutex> lock(mutex_);
    --count_;
    if(count_ == 0) {
        cv_.notify_all();
    }
}

int CountDownLatch::getCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return count_;
}


