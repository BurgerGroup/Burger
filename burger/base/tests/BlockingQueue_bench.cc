#include "burger/base/CountDownLatch.h"
#include "burger/base/BlockingQueue.h"
#include "burger/base/Timestamp.h"

#include <thread>
#include <vector>
class Bench {
public:
    Bench(int numThreads);
    void run();
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

void Bench::run() {
    std::cout << "waiting for count down latch\n";
}


