#include "EventLoop.h"

using namespace burger;
using namespace burger::net;
#include <poll.h>
// 为何这里需要namespace 套着
namespace {
thread_local EventLoop::ptr t_loopInthisThread;
const int kEpollTimesMs = 10000;
int createEventfd() {
    int efd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(efd < 0) {
        ERROR("Failed in eventfd");
        abort();
    }
    return efd;
}
}

EventLoop::EventLoop() : 
    looping_(false),
    quit_(false),
    threadId_(util::gettid()),
    epoll_(util::make_unique<Epoll>()),
    wakeupFd_(createEventfd()) {
}

EventLoop::~EventLoop() {
    DEBUG("EventLoop {} of thread {} destructs", 
        fmt::ptr(this), threadId_)
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInthisThread = nullptr;
}


EventLoop::ptr EventLoop::create() {
    auto el = std::make_shared<EventLoop>();
    el->init();
    return el;
}

void EventLoop::init() {
    TRACE("EventLoop created {} ", fmt::ptr(this));
    if(t_loopInthisThread) {
        CRITICAL("Another EventLoop {} exists in this Thread( tid = {} ) ...", fmt::ptr(t_loopInthisThread), util::gettid()); 
    } else {
        // Can pointer 'this' be a shared pointer?
        // https://stackoverflow.com/questions/37598634/can-pointer-this-be-a-shared-pointer
        t_loopInthisThread = shared_from_this();
    }
    // TODO : 这样到处用shared_from_this 好吗
    // 需要直接用一个成员变量来存还是可以直接用t_loopInthisThread
    wakeupChannel_ = util::make_unique<Channel>(shared_from_this(), wakeupFd_);
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // we are always reading the wakeupfd
    wakeupChannel_->enableReading();
    
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;  // // FIXME: what if someone calls quit() before loop() ?
    TRACE("EventLoop {} start looping", fmt::ptr(this));
    
    while(!quit_) {
        // 每次Epoll::wait就是一次重新填充activeChannels_的过程，所以要clear
        activeChannels_.clear();
        epollWaitReturnTime_ = epoll_->wait(kEpollTimesMs, activeChannels_);
        ++iteration_;
        printActiveChannels();
        // TODO : sort channel by priority
        eventHandling_ = true;
        for(auto channel: activeChannels_) {
            // TODO 这里为啥还要需要个currentActiveChannel
            currentActiveChannel_ = channel;
            currentActiveChannel_->handleEvent(epollWaitReturnTime_);
        }
        currentActiveChannel_ = nullptr;
        eventHandling_ = false;
        doPendingFunctors();
    }
    TRACE("EventLoop {} stop looping", fmt::ptr(this));
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    // TODO: ??
    // There is a chance that loop() just executes while(!quit_) and exits,
    // then EventLoop destructs, then we are accessing an invalid object.
    // Can be fixed using mutex_ in both places.
    // 如果不是当前IO线程调用quit,则需要唤醒wakeup当前IO线程，因为他肯恶搞还阻塞在epoll的位置（EventLoop::loop()）
    // 这样再次循环判断while(!quit_)才能退出循环
    if(!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::assertInLoopThread() {
    if(!isInLoopThread()) {
        abortNotInLoopThread();
    }
}

bool EventLoop::isInLoopThread() const {
    return threadId_ == util::gettid();
}

EventLoop::ptr EventLoop::getEventLoopOfCurrentThread() {
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

void EventLoop::updateChannel(Channel::ptr channel) {
    assert(channel->ownerLoop() == shared_from_this());
    assertInLoopThread();
    epoll_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel::ptr channel) {
    assert(channel->ownerLoop() == shared_from_this());
    assertInLoopThread();
    // TODO 这里不是很清楚
    if(eventHandling_) {
        assert(currentActiveChannel_ == channel ||
            std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
    }
    epoll_->removeChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
    std::cout << "11" << std::endl;
    CRITICAL("EventLoop::abortNotInLoopThread - EventLoop {} was \
        created in threadId_ = {}, and current id = {} ...", 
        fmt::ptr(this), threadId_, util::gettid());
}

void EventLoop::handleRead() {
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

// TODO : 还不够清楚
void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for(const auto& functor: functors) {
        functor();
    }
    callingPendingFunctors_ = false;
}








