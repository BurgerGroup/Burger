#include "EventLoop.h"
#include "EventLoopThread.h"

using namespace burger;
using namespace burger::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb):
    loop_(nullptr),
    exiting_(false),
    threadInitcallback_(cb) {
}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if(loop_ != nullptr) {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop() {
    assert(!thread_.joinable());
    // https://stackoverflow.com/questions/23594244/is-there-a-safe-way-to-have-a-stdthread-as-a-member-of-a-class/
    thread_ = std::thread{&EventLoopThread::threadFunc, this};
    // 这里启动了一个新线程执行threadFunc，旧线程继续在这执行，两者顺序不一定
    // todo : 这里为何如此
    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]{ return loop_ != nullptr; } );
        loop = loop_;
    }
    return loop;
}

// startLoop 和 threadFunc 并发执行
void EventLoopThread::threadFunc() {
    EventLoop loop;
    if(threadInitcallback_) {
        threadInitcallback_(&loop);
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        // loop_ 指针指向一个栈上的对象，threadFunc函数退出后，这个指针就失效了
        loop_ = &loop;
        cv_.notify_one();
    }
    loop.loop();

    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = nullptr;
}

