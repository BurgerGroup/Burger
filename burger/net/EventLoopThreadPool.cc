#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "burger/base/Util.h"

using namespace burger;
using namespace burger::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop):
    baseLoop_(baseLoop),
    started_(false),
    numThreads_(0),
    next_(0) {  
}

EventLoopThreadPool::~EventLoopThreadPool() {
    // Don't delete loop, it's stack variable 
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    EventLoop* loop = baseLoop_;

    // loops_为空，则loop指向baseLoop
    // 如果不为空，按照round-robin(RB, 轮叫)的调度方式选择一个EventLoop
    if(!loopList_.empty()) {
        // round-robin
        loop = loopList_[next_];
        ++next_;
        if(implicit_cast<size_t>(next_) >= loopList_.size()) {
            next_ = 0;
        }
    }
    return loop;
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
    assert(!started_);
    baseLoop_->assertInLoopThread();
    started_ = true;
    for(int i = 0; i < numThreads_; i++) {
        auto eventLoopThread = util::make_unique<EventLoopThread>(cb);
        loopList_.push_back(eventLoopThread->startLoop());  // 启动EventLoopThread线程，在进入事件循环之前，会调用cb
        threadList_.push_back(std::move(eventLoopThread));
    }
    if(numThreads_ == 0 && cb) {
        // 只有一个EventLoop, 在这个EventLoop进入事件循环之前，调用cb
        cb(baseLoop_);
    }
}










