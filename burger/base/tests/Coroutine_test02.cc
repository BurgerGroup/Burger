#include "burger/base/Coroutine.h"
#include "burger/base/Log.h"
#include <vector>
#include <thread>
#include <algorithm>

using namespace burger;

void runInCo() {
    INFO("RUN IN coroutine begin");
    Coroutine::Yield();
    INFO("RUN IN coroutine end");
    Coroutine::Yield();
}


void testCo() {
    INFO("main begin -1");
    {
        INFO("main begin");
        Coroutine::ptr co = std::make_shared<Coroutine>(runInCo);
        co->resume();
        INFO("main after resume");
        co->resume();
        INFO("main after end");
        co->resume();
    }
    INFO("main after end2");
}

int main() {
    LOGGER(); LOG_LEVEL_DEBUG;
    
    testCo();

    return 0;
    
}