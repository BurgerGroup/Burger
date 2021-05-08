#include "burger/net/CoTcpServer.h"
#include "burger/base/Log.h"
#include "burger/net/RingBuffer.h"
#include "burger/net/Scheduler.h"

using namespace burger;
using namespace burger::net;

void connHandler(CoTcpConnection::ptr conn) {
    RingBuffer::ptr buffer = std::make_shared<RingBuffer>();
    while(conn->recv(buffer) > 0) {
        conn->send(buffer);
    }
}

int main() {
    LOGGER(); LOG_LEVEL_TRACE;
    Scheduler sched;
    InetAddress listenAddr(8888);
    CoTcpServer server(&sched, listenAddr, "Echo server");
    server.setConnectionHandler(connHandler);
    server.start();
    sched.wait();
    return 0;
}