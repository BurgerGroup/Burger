#include "EventLoop.h"
#include "Epoll.h"
#include "TimerId.h"
#include "TimerQueue.h"
#include "SocketsOps.h"
#include "Channel.h"
#include <signal.h>

using namespace burger;
using namespace burger::net;

namespace {
thread_local EventLoop* t_loopInthisThread = nullptr;

const int kEpollTimesMs = 10000;  // epoll 超时10 s

#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe {
public:
    IgnoreSigPipe() {
        ::signal(SIGPIPE, SIG_IGN);
        TRACE("Ignore SIGPIPE");
    }
};
#pragma GCC diagnostic error "-Wold-style-cast"
IgnoreSigPipe initObj;

}  // namespace


EventLoop::EventLoop() : 
    looping_(false),
    quit_(false),
    eventHandling_(false),
    iteration_(0),
    threadId_(util::tid()),
    epoll_(util::make_unique<Epoll>(this)),
    timerQueue_(util::make_unique<TimerQueue>(this)),
    wakeupFd_(sockets::createEventfd()),
    wakeupChannel_(util::make_unique<Channel>(this, wakeupFd_)),
    currentActiveChannel_(nullptr),
    threadLocalLoopPtr_(&t_loopInthisThread) {
    TRACE("EventLoop created {}", fmt::ptr(this));
    if(t_loopInthisThread) {
        CRITICAL("Another EventLoop {} exists in this Thread( tid = {} ) ...", fmt::ptr(t_loopInthisThread), util::tid()); 
    } else {
        // Can pointer 'this' be a shared pointer?
        // https://stackoverflow.com/questions/37598634/can-pointer-this-be-a-shared-pointer
        t_loopInthisThread = this;
    }
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleWakeupFd, this));
    // we are always reading the wakeupfd
    wakeupChannel_->enableReading();  
}

EventLoop::~EventLoop() {
    DEBUG("EventLoop {} of thread {} destructs", 
        fmt::ptr(this), threadId_)
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInthisThread = nullptr;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;  // FIXME: what if someone calls quit() before loop() ?
    TRACE("EventLoop {} start looping", fmt::ptr(this));
    
    while(!quit_) {
        // 每次Epoll::wait就是一次重新填充activeChannels_的过程，所以要clear
        activeChannels_.clear();
        epollWaitReturnTime_ = epoll_->wait(kEpollTimesMs, activeChannels_);
        ++iteration_;
        // printActiveChannels();  // for test
        // TODO : sort channel by priority
        eventHandling_ = true;
        for(auto channel: activeChannels_) {
            // currentActiveChannel_ 在removeChannel()中需要做判断 todo 理
            currentActiveChannel_ = channel;
            currentActiveChannel_->handleEvent(epollWaitReturnTime_);
        }
        currentActiveChannel_ = nullptr;
        eventHandling_ = false;
        doQueueInLoopFuncs();
    }
    TRACE("EventLoop {} stop looping", fmt::ptr(this));
    looping_ = false;
}

// 可以跨线程调用
void EventLoop::quit() {
    quit_ = true;
    // TODO: ??
    // There is a chance that loop() just executes while(!quit_) and exits,
    // then EventLoop destructs, then we are accessing an invalid object.
    // Can be fixed using mutex_ in both places.
    // 如果不是当前IO线程调用quit,则需要唤醒wakeup当前IO线程，因为他可能还阻塞在epoll wait的位置（EventLoop::loop()）
    // 相当于触发了个可读事件, 这样再次循环判断while(!quit_)才能退出循环
    if(!isInLoopThread()) {
        wakeup();  
    }
}

// 这是一种编程思想，当其他线程需要通过这个线程的资源来执行任务的时候，
// 并不是直接再其他线程中访问资源调用函数
// 这样就会造成资源的竞争，需要加锁保证，而现在我们让当前线程
// 为其他线程提供一个接口，其他线程将要执行的任务用这个接口交给当前线程
// 这样当前线程统一处理自己的资源，而不用加锁，唯一需要加锁的地方就是
// 通过接口添加任务的任务队列这个地方，大大减小了锁粒度
void EventLoop::runInLoop(const Func& func) {
    if(isInLoopThread()) {
        func();   // 如果在当前IO线程中调用，则同步调用
    } else {
        queueInLoop(func);   // 如果在其他线程中调用该函数，则异步调用，用queueInLoop添加到任务队列中
    }
}

void EventLoop::runInLoop(Func&& func) {
    if(isInLoopThread()) {
        func();
    } else {
        queueInLoop(std::move(func));
    }
}
// 加入队列中，等待被执行，该函数可以跨线程调用，即其他线程可以给当前线程添加任务
void EventLoop::queueInLoop(const Func& func)  {
    queueFuncs_.enqueue(func);  // 无锁队列入队
    // 如果不是当前线程(可能阻塞在wait)，需要唤醒 
    // 或者是当前线程但是在正在处理队列中的任务(使得处理完当前队列中的元素后立即在进行下一轮处理，因为在这里又添加了任务)需要唤醒
    // 只有当前IO线程的事件回调中调用queueInLoop才不需要唤醒(因为执行完handleEvent会自然执行loop()doQueueInLoopFuncs)
    if(!isInLoopThread()) {  // todo !looping
        wakeup();
    }
}

void EventLoop::queueInLoop(Func&& func)  {
    queueFuncs_.enqueue(std::move(func));
    if(!isInLoopThread()) {  // todo !looping
        wakeup();
    }
}

TimerId EventLoop::runAt(Timestamp time, TimerCallback timercb) {
    return timerQueue_->addTimer(std::move(timercb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback timercb) {
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, std::move(timercb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback timercb) {
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(std::move(timercb), time, interval);
}

void EventLoop::cancel(TimerId timerId) {
    return timerQueue_->cancel(timerId);
}

void EventLoop::assertInLoopThread() {
    if(!isInLoopThread()) {
        abortNotInLoopThread();
    }
}

bool EventLoop::isInLoopThread() const {
    return threadId_ == util::tid();  
}

// 如果当前线程不是IO线程则为nullptr
EventLoop* EventLoop::getEventLoopOfCurrentThread() {
    return t_loopInthisThread;
}

/**
 * @brief eventfd事件类似于管道，可以更简单的实现线程间事件通信
 * 唤醒就是往这个事件的fd写一个uint64_t
 */
void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = sockets::write(wakeupFd_, &one, sizeof(one));
    if(n != sizeof(one)) {
        ERROR("EventLoop::wakeup() writes {} bytes instead of 8", n);
    }
} 

void EventLoop::moveToCurrentThread() {
    if (isRunning()) {
        CRITICAL("EventLoop cannot be moved when running");
    }
    if (isInLoopThread()) {
        WARN("This EventLoop is already in the current thread");
        return;
    }
    if (t_loopInthisThread) {
        CRITICAL("There is already an EventLoop in this thread, you cannot move another in");
    }
    // *threadLocalLoopPtr_ = nullptr;
    t_loopInthisThread = this;
    threadLocalLoopPtr_ = &t_loopInthisThread;
    threadId_ = util::tid();
}

bool EventLoop::isRunning() {
    return looping_ && (!quit_);
}

void EventLoop::updateChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    epoll_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    // todo 需要再这个时序上理一下
    if(eventHandling_) {
        assert(currentActiveChannel_ == channel ||
            std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
    }
    epoll_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return epoll_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
    CRITICAL("EventLoop::abortNotInLoopThread - EventLoop {} was \
        created in threadId_ = {}, and current id = {} ...", 
        fmt::ptr(this), threadId_, util::tid());
}

void EventLoop::handleWakeupFd() {
    uint64_t one = 1;
    ssize_t n = sockets::read(wakeupFd_, &one, sizeof(one));
    if(n != sizeof(one)) {
        ERROR("EventLoop::handleRead reads {} bytes instead of 8 ", n);
    }
}

void EventLoop::printActiveChannels() const {
    for(const auto& channel: activeChannels_) {
        TRACE("[ {} ]", channel->reventsToString());
    }
}
// 1. 不是简单地在临界区内依次调用Functor，
// 而是把回调列表swap到functors中，这样一方面减小了临界区的长度
//（意味着不会阻塞其它线程的queueInLoop()添加任务到pendingFunctors_），另一方面，也避免了死
//锁（因为Functor可能再次调用queueInLoop()添加任务到pendingFunctors_）

//2. 由于doPendingFunctors()调用的Functor可能再次调用queueInLoop(cb)添加任务到pendingFunctors_，这时，
// queueInLoop()就必须wakeup()，否则新增的cb可能就不能及时调用了

// 3. 没有反复执行doPendingFunctors()直到pendingFunctors为空，
// 这是有意的，否则IO线程可能陷入死循环，无法处理IO事件。
void EventLoop::doQueueInLoopFuncs() {
    callingQueueFuncs_ = true;
    // todo : the destructor for the Func may itself insert a new entry into the
    // queue 从而造成死循环 p294
    while(!queueFuncs_.empty()) {
        Func func;
        while(queueFuncs_.dequeue(func)) {
            func();
        }
    }
    callingQueueFuncs_ = false;
} 








