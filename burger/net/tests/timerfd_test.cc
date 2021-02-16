#include "burger/net/Channel.h"
#include "burger/net/EventLoop.h"
#include <iostream>
#include <functional>
#include <sys/timerfd.h>

/**
 * bug need to fix
 */
using namespace burger;
using namespace burger::net;

EventLoop* g_loop;
int timerfd;

void timeout(Timestamp receiveTime) {
    std::cout << "Timeout!\n";
    uint64_t howmany;
    // 采用LT，不把他读走的话就会一直触发
    ::read(timerfd, &howmany, sizeof(howmany));
    g_loop->quit();
}

int main() {
    if (!Logger::Instance().init("log", "logs/test.txt", spdlog::level::trace)) {
		return 1;
	}
    EventLoop loop;
    g_loop = &loop;
    timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Channel channel(&loop, timerfd);
    channel.setReadCallback(std::bind(timeout, std::placeholders::_1));
    channel.enableReading();

    struct itimerspec howlong;
    bzero(&howlong, sizeof(howlong));
    // howlong.it_interval = 0; // 这里已经被清零0了，所以是一次性的
    howlong.it_value.tv_sec = 1;
    ::timerfd_settime(timerfd, 0, &howlong, NULL);
    loop.loop();
    ::close(timerfd);
}