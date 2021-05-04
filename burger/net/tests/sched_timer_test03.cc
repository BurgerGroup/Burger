#include "burger/net/Scheduler.h"
#include "burger/net/TimerId.h"
#include "burger/base/Log.h"
#include "burger/base/Util.h"

#include <unistd.h>
#include <iostream>
#include <functional>

using namespace burger;
using namespace burger::net;

// todo : timer 实验03

int cnt = 0;
Scheduler* g_sched;

void print(const std::string& msg) {
    std::cout << "msg " << msg << " : " << Timestamp::now().toFormatTime() << std::endl;
    if(++cnt == 3) {
        g_sched->stop();
    }
}

void stopSched() {
    g_sched->stop();
}

int main() {
    LOGGER(); LOG_LEVEL_TRACE;

    Scheduler sched;
    g_sched = &sched;
    sched.startAsync();
    std::cout << "main" << std::endl;

    sched.runEvery(1, std::bind(print, "every1"));

    sched.wait();
    
    return 0;
}