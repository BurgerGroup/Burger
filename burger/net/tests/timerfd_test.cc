#include "burger/net/Channel.h"
#include "burger/net/EventLoop.h"
#include <iostream>
#include <functional>
#include <sys/timerfd.h>

using namespace burger;
using namespace burger::net;

EventLoop::ptr g_loop;
int timerfd;

void timeout(Timestamp receiveTime) {
    std::cout << "Timeout!\n";
    uint64_t howmany;
    ::read(timerfd, &howmany, sizeof(howmany));
    g_loop->quit();
}

int main() {
    if (!Logger::Instance().init("log", "logs/test.log", spdlog::level::trace)) {
        std::cout << "Logger init error" << std::endl;
		return 1;
	}
    auto loop = EventLoop::create();
    g_loop = loop;
    
    timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    
}