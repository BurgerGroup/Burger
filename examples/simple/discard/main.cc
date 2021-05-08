#include "discard.h"

#include "burger/base/Log.h"
#include "burger/net/InetAddress.h"
#include "burger/net/Scheduler.h"

int main() {
    LOGGER(); LOG_LEVEL_INFO;
    Scheduler sched;

    InetAddress listenAddr(8888);
    DiscardServer server(&sched, listenAddr);
    server.start();
    
    sched.wait();
}