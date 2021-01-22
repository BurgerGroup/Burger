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
    if (!Logger::Instance().init("log", "logs/test.txt", spdlog::level::trace)) {
		return 1;
	}
    std::cout << "ThreadFunc : pid = " << getpid() 
        << " tid = " << util::gettid() << std::endl; 
    auto loop = EventLoop::create();
    std::thread t(threadFunc);
    loop->loop();
    t.join();
    return 0;
}