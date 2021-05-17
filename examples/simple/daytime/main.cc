#include "daytime.h"
#include "burger/net/Scheduler.h"
#include "burger/base/Log.h"

int main() {
    LOGGER(); LOG_LEVEL_INFO;
    Scheduler sched;
    InetAddress listenAddr(8888);
    DaytimeServer server(&sched, listenAddr);
    server.start();
    sched.wait();
    return 0;
}