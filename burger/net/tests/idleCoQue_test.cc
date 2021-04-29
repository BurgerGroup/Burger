#include "burger/net/Scheduler.h"
#include "burger/base/Log.h"
#include <chrono>
#include <thread>
#include <iostream>
using namespace burger;
using namespace burger::net;

int gCount = 0;
Scheduler* gSched = nullptr;

void printTask() {
    INFO("In task: {}", gCount++);
    if(gCount == 10) {
        INFO("{} tasks done.stop!", gCount);
        gSched->stop();
    }
}

int main() {
    LOGGER(); 
    LOG_LEVEL_TRACE;
    Scheduler s;
    gSched = &s;
    s.startAsync();
    for(int i = 0;i < 15;++i) {
        std::cout << "add task " << i << std::endl;
        s.addTask(printTask, "printTask");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    s.wait();
    return 0;
}