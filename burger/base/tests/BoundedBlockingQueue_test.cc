#include <thread>
#include "burger/base/BoundedBlockingQueue.h"
#include "burger/base/CountDownLatch.h"
#include <thread>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unistd.h> // getpid
class Test {
public:
    Test(int numThreads);
    void run(int times);
    void joinAll();

private:
    void ThreadFunc();

private:
    burger::BoundedBlockingQueue<std::string> queue_;
    burger::CountDownLatch latch_;
    std::vector<std::thread> threads_;
};

Test::Test(int numThreads) : queue_(20), latch_(numThreads), threads_(numThreads) {
    for(int i = 0; i < numThreads; i++) {
        threads_[i] = std::thread(&Test::ThreadFunc, this);
    }
}

void Test::run(int times) {
    std::cout << "waiting for count down latch\n";
    latch_.wait();
    std::cout << "all threads started" << std::endl;
    for(int i = 0; i < times; i++) {
        std::string data = "hello  " + std::to_string(i);
        queue_.put(data);
        std::cout << " tid = " << std::this_thread::get_id()
            << "\t PUT data = " << data << "\t size = " << queue_.size() << std::endl;
    }
}

void Test::joinAll() {
    for(size_t i = 0; i < threads_.size(); i++) {
        queue_.put("stop");
    }
    std::for_each(threads_.begin(), threads_.end(), std::mem_fn(&std::thread::join));
}

void Test::ThreadFunc() {
    std::thread::id threadId = std::this_thread::get_id();
    std::cout << "tid = " << threadId << std::endl;
    latch_.countDown();
    bool running = true;
    while(running) {
        std::string d(queue_.take());
        std::cout << " tid = " << threadId 
            << "\t get data = " << d << "\t size = " << queue_.size() << std::endl;
        running = (d != "stop");
    }
    std::cout << "tid = " << threadId << " stopped" << std::endl;
}

int main() {
    std::cout << "pid = " << ::getpid() << "\t tid = "
        << std::this_thread::get_id() << std::endl;
    Test t(5);
    t.run(100);
    t.joinAll();
}


