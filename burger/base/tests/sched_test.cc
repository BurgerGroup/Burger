#include "burger/base/scheduler.h"
#include "burger/base/Util.h"

using namespace burger;

void test_co() {
    static int s_count = 5;
    INFO("test in fiber s_count= {}", s_count);

    sleep(1);
    if(--s_count >= 0) {
        Scheduler::GetSched()->schedule(&test_co, util::tid());
    }
}

int main(int argc, char** argv) {
    LOGGER(); LOG_LEVEL_DEBUG;
    INFO("main");
    Scheduler sc(3, false, "test");
    sc.start();
    sleep(2);
    INFO("schedule");
    sc.schedule(&test_co);
    sc.stop();
    INFO("OVER");
    return 0;
}