/*
用于空loop版本测试
*/

#include "burger/net/EventLoop.h"
#include <thread>
#include <iostream>
#include <unistd.h>
using namespace burger;
using namespace burger::net;

void threadFunc() {
    std::cout << "ThreadFunc : pid = " << getpid() 
        << " tid = " << util::gettid() << std::endl; 
    EventLoop loop;
    loop.loop();
}

int main() {
    std::cout << "ThreadFunc : pid = " 
    << getpid() 
        << " tid = " << util::gettid() << std::endl; 
    EventLoop loop;
    std::thread t(threadFunc);
    loop.loop();
    t.join();
    return 0;
}