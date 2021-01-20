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
    threadId_(std::this_thread::get_id()) {
    spdlog::set_level(spdlog::level::trace);
    spdlog::set_pattern("%Y-%m-%d %H:%M:%S [%l] [%t] - <%s>|<%#>|<%!>,%v");
}

EventLoop::ptr EventLoop::create() {
    std::shared_ptr<EventLoop> el = std::make_shared<EventLoop>();
    el->init();
    return el;
}

void EventLoop::init() {
    spdlog::trace("EventLoop created {} in thread ( {} )", fmt::ptr(this), util::threadIdToStr(threadId_));
    if(t_loopInthisThread) {
        spdlog::critical("Another EventLoop {} exists in this Thread( tid = {} ) ...", fmt::ptr(t_loopInthisThread), util::threadIdToStr(threadId_)); 
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
    spdlog::trace("EventLoop {} start looping", fmt::ptr(this));
    spdlog::trace("EventLoop {} start looping", fmt::ptr(this));
    ::poll(NULL, 0, 5*1000);
    spdlog::trace("EventLoop {} stop looping", fmt::ptr(this));
    looping_ = false;
}

void EventLoop::assertInLoopThread() {
    if(!isInLoopThread()) {
        abortNotInLoopThread();
    }
}

bool EventLoop::isInLoopThread() const {
    return threadId_ == std::this_thread::get_id();
}

EventLoop::ptr EventLoop::getEventLoopOfCurrentThread() {
    return t_loopInthisThread;
}

void EventLoop::abortNotInLoopThread() {
    std::cout << "11" << std::endl;
    spdlog::critical("EventLoop::abortNotInLoopThread - EventLoop {} was \
        created in threadId_ = {}, and current id = {} ...", 
        fmt::ptr(this), util::threadIdToStr(threadId_), util::threadIdToStr((std::this_thread::get_id())));
}






