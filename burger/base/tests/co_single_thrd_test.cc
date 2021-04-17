#include "burger/base/coroutine.h"
#include "burger/base/Log.h"
#include "burger/base/scheduler.h"
#include <vector>
#include <thread>
#include <algorithm>

using namespace burger;

void runInCo() {
    INFO("RUN IN coroutine begin");
    Coroutine::YieldToHold();
    INFO("RUN IN coroutine end");
    Coroutine::YieldToHold();
}


void testCo() {
    INFO("main begin -1");
    {
        Coroutine::GetThis();  
        INFO("main begin");
        Coroutine::ptr co = std::make_shared<Coroutine>(runInCo);
        co->swapIn();
        INFO("main after swapIn");
        co->swapIn();
        INFO("main after end");
        co->swapIn();
    }
    INFO("main after end2");
}

int main() {
    LOGGER(); LOG_LEVEL_DEBUG;
    
    testCo();

    return 0;
    
}