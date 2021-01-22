#include "EventLoop.h"

using namespace burger;
using namespace burger::net;
#include <poll.h>
// 为何这里需要namespace 套着
namespace {
    thread_local EventLoop::ptr t_loopInthisThread;
}

EventLoop::EventLoop() : 
    looping_(false),
    threadId_(util::gettid()) {
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
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    TRACE("EventLoop {} start looping", fmt::ptr(this));
    ::poll(NULL, 0, 5*1000);
    TRACE("EventLoop {} stop looping", fmt::ptr(this));
    looping_ = false;
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

void EventLoop::abortNotInLoopThread() {
    std::cout << "11" << std::endl;
    CRITICAL("EventLoop::abortNotInLoopThread - EventLoop {} was \
        created in threadId_ = {}, and current id = {} ...", 
        fmt::ptr(this), threadId_, util::gettid());
}






