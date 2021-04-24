#include "burger/net/Scheduler.h"
#include "burger/base/Log.h"
#include "burger/base/Util.h"

using namespace burger; 
using namespace burger::net;

int main() {
    // todo : sigleton error
    // auto sched = Singleton<Scheduler>::Instance();
    LOGGER(); LOG_LEVEL_TRACE;
    auto sched = util::make_unique<Scheduler>();
    sched->addTask([]() {
        DEBUG("before sleep");
        sleep(5);
        DEBUG("after sleep");
    }, "sleep func");
    sched->start();
    return 0;
}