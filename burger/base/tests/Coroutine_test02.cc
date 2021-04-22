#include "burger/base/Coroutine.h"
#include "burger/base/Log.h"
#include <vector>
#include <thread>
#include <algorithm>

using namespace burger;

void runInCo() {
    INFO("RUN IN coroutine begin");
    Coroutine::SwapOut();
    INFO("RUN IN coroutine end");
    Coroutine::SwapOut();
}


void testCo() {
    INFO("main begin -1");
    {
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