#include "burger/net/EventLoop.h"
#include <iostream>
#include <memory>
#include <thread>
/**
 * 跨线程调用测试
 * 负面测试
 */

using namespace burger;
using namespace burger::net;

EventLoop* g_loop;

void ThreadFunc() {
    g_loop->loop(); 
    // 在当前线程调用另一个线程的EventLoop::loop, 程序终止，FATAL
}

int main() {
    EventLoop loop;
    g_loop = &loop;
    std::thread t1(ThreadFunc);
    t1.join();
    return 0;
}