#include <iostream>
#include "burger/net/EventLoop.h"
#include "burger/net/EventLoopThread.h"

#include <chrono>
#include <stdlib.h>
using namespace burger;
using namespace burger::net;

void runInThread() {
    std::cout << "runInThread() : pid = " << ::getpid() 
        << " tid  = " << util::gettid() << std::endl;
}

int main() {
    std::cout << "main() : pid = " << ::getpid() 
        << " tid  = " << util::gettid() << std::endl;
    EventLoopThread loopThread;
    EventLoop* loop = loopThread.startLoop();  // 指针指向的是栈上的对象
    // 异步调用runInThread, 即将runInthread 添加到loop对下个所在的IO线程，让该IO线程执行
    loop->runInLoop(runInThread);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // runAfter 内部也调用了runInLoop,所以这里也是异步调用
    loop->runAfter(2, runInThread);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    loop->quit();
    std::cout << "Exit main()\n";
}
