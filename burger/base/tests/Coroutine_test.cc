#include "burger/base/coroutine.h"
#include <vector>
#include <thread>
#include <algorithm>
using namespace burger;

void runInCo() {
    INFO("RUN IN coroutine begin");
    Coroutine::YeildToHold();
    INFO("RUN IN coroutine end");
    Coroutine::YeildToHold();
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
    std::vector<std::thread> threads;
    for(int i = 0; i < 3; i++) {
        threads.push_back(std::thread(testCo));
    }
    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
    return 0;
    
}