#include "burger/net/Scheduler.h"
#include "burger/base/Log.h"
#include <chrono>
#include <thread>
using namespace burger;
using namespace burger::net;

// 优雅退出实验

Scheduler* g_sched;

void foo() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    INFO("IN foo");
    g_sched->stop();
    INFO("leave foo");
}

int main() {
    LOGGER(); LOG_LEVEL_TRACE;
    Scheduler sched;
    g_sched = &sched;
    sched.startAsync();
    sched.addTask(foo, "foo");
    std::cout << "prior to foo" << std::endl;
    sched.wait();
    return 0;
}