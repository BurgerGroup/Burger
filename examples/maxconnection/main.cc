#include "echo.h"

#include "burger/base/Log.h"
#include "burger/net/InetAddress.h"
#include "burger/net/Scheduler.h"


// 回设服务器，最大连接数为2

int main(int argc, char* argv[]) {
    LOGGER(); LOG_LEVEL_INFO;
    Scheduler sched;
    int maxConnections = 2;
    if (argc > 1) {
        maxConnections = atoi(argv[1]);
    }
    INFO("maxConnections : {}", maxConnections);
    InetAddress listenAddr(8888);
    EchoServer server(&sched, listenAddr, maxConnections);
    
    server.start();
    sched.wait();
}