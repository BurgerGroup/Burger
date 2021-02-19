#include "burger/net/EventLoop.h"
#include <iostream>

/**
 * @brief 当线程调用例子
 * main(): pid = 123495 ,flag = 0
run1(): pid = 123495 ,flag =  1    
run2(): pid = 123495 ,flag = 1  run1直接runInLoop，在本线程，立马同步调用，所以还没到2
run3(): pid = 123495 ,flag = 2  run2 queueInLoop(run3), 已经到2了
run4(): pid = 123495 ,flag = 3  还没执行run4，就已经是3了
main(): pid = 123495 ,flag = 3
 */
using namespace burger;
using namespace burger::net;

EventLoop* g_loop;
int g_flag = 0;

void run4() {
    std::cout << "run4(): pid = " << getpid() << " ,flag = " << g_flag << std::endl;
    g_loop->quit();
}

void run3() {
    std::cout << "run3(): pid = " << getpid() << " ,flag = " << g_flag << std::endl;
    g_loop->runAfter(3, run4);
    g_flag = 3;
}

void run2() {
    std::cout << "run2(): pid = " << getpid() << " ,flag = " << g_flag << std::endl;
    g_loop->queueInLoop(run3);
}

void run1() {
    g_flag = 1;
    std::cout << "run1(): pid = " << getpid() << " ,flag = " << g_flag << std::endl;
    g_loop->runInLoop(run2);
    g_flag = 2;
}

int main() {
    std::cout << "main(): pid = " << getpid() << " ,flag = " << g_flag << std::endl;
    EventLoop loop;
    g_loop = &loop;

    loop.runAfter(2, run1);
    loop.loop();
    std::cout << "main(): pid = " << getpid() << " ,flag = " << g_flag << std::endl;
}