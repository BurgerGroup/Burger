#include "burger/base/CountDownLatch.h"
#include "burger/base/BlockingQueue.h"
#include "burger/base/Timestamp.h"
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>
#include <unistd.h> // getpid
#include <map>

class Bench {
public:
    Bench(int numThreads);
    void run(int times);
    void joinAll();
private:
    void ThreadFunc();

private:
    burger::BlockingQueue<burger::Timestamp> queue_;
    burger::CountDownLatch latch_;
    std::vector<std::thread> threads_;

};

Bench::Bench(int numThreads) : latch_(numThreads), threads_(numThreads) {
    for(int i = 0; i < numThreads; i++) {
        threads_[i] = std::thread(&Bench::ThreadFunc, this); 
    }
}

void Bench::run(int times) {
    std::cout << "waiting for count down latch\n";
    latch_.wait();
    std::cout << "all threads started\n" << std::endl;
    for(int i = 0; i < times; i++) {
        queue_.put(burger::Timestamp::now());
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

void Bench::joinAll() {
    for(size_t i = 0; i < threads_.size(); i++) {
        queue_.put(burger::Timestamp::invalid());
    }
    std::for_each(threads_.begin(), threads_.end(), std::mem_fn(&std::thread::join));
}

void Bench::ThreadFunc() {
    std::thread::id threadId = std::this_thread::get_id();
    std::cout << "tid = " << threadId << std::endl;
    std::map<int, int> delays;
    latch_.countDown();
    bool running = true;
    while(running) {
        burger::Timestamp t(queue_.take());
        burger::Timestamp now(burger::Timestamp::now());
        if(t.valid()) {
            int delay = static_cast<int>(timeDifference(now, t) * 1000000);
            ++delays[delay];
        }
        running = t.valid();
    }
    std::cout << "tid = " << threadId << " stopped" << std::endl;
    for(auto it = delays.begin(); it != delays.end(); ++it) {
        std::cout << "tid = " << threadId << " delay = " 
            << it->first << " count = " << it->second << std::endl;
    }
}

int main(int argc, char* argv[]) {
    int threads = argc > 1 ? atoi(argv[1]) : 1;
    Bench t(threads);
    t.run(10000);
    t.joinAll();
}
