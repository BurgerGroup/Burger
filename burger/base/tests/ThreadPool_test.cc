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
    burger::Threadpool pool("MainThreadPool");
    pool.start(5);
    pool.run(print);
    pool.run(print);
    for(int i = 0; i < 100; i++) {
        // right value 
        // 开启run里的INFO可做实验观察
        pool.run(std::bind(printStr, std::string("task ") + std::to_string(i)));
    }   
    for(int i = 101; i < 200; i++) {
        burger::Threadpool::Task task = std::bind(printStr, std::string("task ") + std::to_string(i));
        pool.run(task);  // const reference , 注意auto在这里就是right value
    }   
    burger::CountDownLatch latch(1);
    pool.run(std::bind(&burger::CountDownLatch::countDown, &latch));
    latch.wait();
    std::cout << "CountDownLatch::countDown() is handled...\n";
    pool.stop();
}