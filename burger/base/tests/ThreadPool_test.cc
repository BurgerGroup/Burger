#include "burger/base/ThreadPool.h"
#include "burger/base/CountDownLatch.h"
#include <iostream>
#include <thread>
#include <functional>
void print() {
    std::cout << "tid = " << std::this_thread::get_id() << std::endl;
}

void printStr(const std::string& str) {
    std::cout << "tid = " << std::this_thread::get_id() << " : "<< str << std::endl;
}

int main() {
    // TODO : RAII?
    burger::Threadpool pool("MainThreadPool");
    pool.start(5);
    pool.run(print);
    pool.run(print);
    for(int i = 0; i < 100; i++) {
        pool.run(std::bind(printStr, std::string("task ") + std::to_string(i)));
    }   
    burger::CountDownLatch latch(1);
    pool.run(std::bind(&burger::CountDownLatch::countDown, &latch));
    latch.wait();
    std::cout << "CountDownLatch::countDown() is handled...\n";
    pool.stop();
}