#include "burger/net/Scheduler.h"
#include "burger/net/TimerId.h"
#include "burger/base/Log.h"

#include <unistd.h>
#include <iostream>


using namespace burger;
using namespace burger::net;

// todo : timer 实验

void test() {
    std::cout << "test timer" << std::endl;
}



int main() {
    LOGGER(); LOG_LEVEL_TRACE;
    Scheduler sched;
    sched.startAsync();
    auto co = std::make_shared<Coroutine>(test, "timerTest");
    TimerId timerid = sched.runEvery(2, co);
    sleep(5);
    sched.cancel(timerid);
    sched.wait();
    return 0;
}