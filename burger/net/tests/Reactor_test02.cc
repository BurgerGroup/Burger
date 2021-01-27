#include "burger/net/EventLoop.h"
#include <iostream>
#include <memory>
#include <thread>
/**
 * 跨线程调用测试
 */

using namespace burger;
using namespace burger::net;

std::shared_ptr<EventLoop> g_loop;

void ThreadFunc() {
    g_loop->loop(); 
    // 在当前线程调用另一个线程的EventLoop::loop, 程序终止，FATAL
}

int main() {
    if (!Logger::Instance().init("log", "logs/test.log", spdlog::level::trace)) {
        std::cout << "Logger init error" << std::endl;
		return 1;
	}
    auto loop = std::make_shared<EventLoop>();
    g_loop = loop;
    std::thread t1(ThreadFunc);
    t1.join();
    return 0;
}