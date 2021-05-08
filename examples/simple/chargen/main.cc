#include "chargen.h"
#include "burger/net/Scheduler.h"
#include "burger/base/Log.h"

int main() {
    LOGGER(); LOG_LEVEL_INFO;
    Scheduler sched;

    InetAddress listenAddr(8888);
    ChargenServer server(&sched, listenAddr);
    
    server.start();
    sched.wait();
}