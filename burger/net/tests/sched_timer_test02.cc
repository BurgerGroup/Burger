#include "burger/net/Scheduler.h"
#include "burger/net/TimerId.h"
#include "burger/base/Log.h"
#include "burger/base/Util.h"

#include <unistd.h>
#include <iostream>
#include <functional>

using namespace burger;
using namespace burger::net;

// todo : timer 实验02

int cnt = 0;
Scheduler* g_sched;

void printTid() {
    std::cout << "pid = " << getpid() << " tid = " << util::gettid() << std::endl;
    std::cout << "now : " << Timestamp::now().toString() << std::endl;
}

void print(const std::string& msg) {
    std::cout << "msg " << msg << " : " << Timestamp::now().toString() << std::endl;
    if(++cnt == 14) {
        g_sched->stop();
    }
}

void cancel(TimerId timer, const std::string& name) {
    g_sched->cancel(timer);
    std::cout << name <<" cancelled at " << Timestamp::now().toString() << std::endl;
}

void stopSched() {
    g_sched->stop();
}

int main() {
    LOGGER(); LOG_LEVEL_DEBUG;
    printTid();

    Scheduler sched;
    g_sched = &sched;
    sched.startAsync();
    std::cout << "main" << std::endl;
    sched.runAfter(1, std::bind(print, "once 1"));
    sched.runAfter(1.5, std::bind(print, "once 1.5"));
    sched.runAfter(2.5, std::bind(print, "once 2.5"));
    sched.runAfter(3.5, std::bind(print, "once 3.5"));
    TimerId t45 = sched.runAfter(4.5, std::bind(print, "once 4.5"));
    // 4.2时把4.5注销掉
    sched.runAfter(4.2, std::bind(cancel, t45, "t45"));
    // 已经取消，这里已没什么效果
    sched.runAfter(4.8, std::bind(cancel, t45, "t45"));
    sched.runEvery(2, std::bind(print, "every2"));
    TimerId t3 = sched.runEvery(3, std::bind(print, "every3"));
    sched.runAfter(9.001, std::bind(cancel, t3, "t3"));
    sched.runAfter(10, stopSched);

    sched.wait();
    return 0;
}