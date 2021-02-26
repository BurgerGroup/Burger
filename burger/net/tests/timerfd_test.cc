#include "burger/net/Channel.h"
#include "burger/net/EventLoop.h"
#include <iostream>
#include <functional>
#include <sys/timerfd.h>

/**
 * @brief 这里只是个小测试，但这个测试是有问题的
 * Channel不能直接这样用，这样会导致Channel没下树，析构会出问题
 * 然后abort 导致EventLoop无法析构，从而无法导致里面的wakeupfd,timerfd等下树再析构
 * 
 * timerfd是在定时器超时的一刻变得可读，就很方便融入epoll框架，以统一的方式处理IO事件和超时事件
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

    EventLoop loop;
    g_loop = &loop;
    // 以下这些都封装成TimerQueue
    timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Channel channel(&loop, timerfd);
    channel.setReadCallback(std::bind(timeout, std::placeholders::_1));
    channel.enableReading();

    struct itimerspec howlong;
    bzero(&howlong, sizeof(howlong));
    // howlong.it_interval = 0; // 这里已经被清零0了，所以是一次性的
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &howlong, NULL);
    loop.loop();
    // 我们这里需要手动给他下树
    channel.disableAll();
    channel.remove();
    ::close(timerfd);
}