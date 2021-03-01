#include "chargen.h"
#include "burger/net/EventLoop.h"

int main() {
    // LOG_LEVEL_INFO;
    INFO("pid = {}", ::getpid());
    EventLoop loop;
    InetAddress listenAddr(8888);
    ChargenServer server(&loop, listenAddr, true);
    server.start();
    loop.loop();
}