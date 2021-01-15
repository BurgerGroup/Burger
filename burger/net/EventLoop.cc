/*
 * @Author: Shiyu Yi
 * @Github: https://github.com/chanchann
 */

#include "EventLoop.h"

using namespace burger;
using namespace burger::net;


namespace {
__thread std::unique_ptr<EventLoop> t_loopInThisThread;
}

EventLoop::EventLoop() : looping_(false),
                        threadId_(std::this_thread::get_id()) {
    LOG_FATAL << 
}
