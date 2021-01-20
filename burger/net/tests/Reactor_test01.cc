#include "burger/net/EventLoop.h"
#include <thread>
#include <iostream>
#include <unistd.h>
using namespace burger;
using namespace burger::net;

void threadFunc() {
    std::cout << "ThreadFunc : pid = " << getpid() 
        << " tid = " << std::this_thread::get_id() << std::endl; 
    auto loop = EventLoop::create();
    loop->loop();
}

int main() {
    std::cout << "ThreadFunc : pid = " << getpid() 
        << " tid = " << std::this_thread::get_id() << std::endl; 
    auto loop = EventLoop::create();
    std::thread t(threadFunc);
    loop->loop();
    t.join();
    return 0;
}