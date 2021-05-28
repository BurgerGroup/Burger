#include "burger/net/CoTcpServer.h"
#include "burger/base/Log.h"
#include "burger/net/RingBuffer.h"
#include "burger/net/Scheduler.h"

using namespace burger;
using namespace burger::net;

void connHandler(CoTcpConnection::ptr conn) {
    conn->setTcpNoDelay(true);
    RingBuffer::ptr buffer = std::make_shared<RingBuffer>();
    while(conn->recv(buffer) > 0) {
        conn->send(buffer);
    }
}

int main(int argc, char* argv[]) {
    uint16_t port = 8888;
    int threadNum = 4;

    if (argc != 1 && argc != 3) {
        printf("Usage: %s <port> <thread-num>\n", argv[0]);
        printf("  e.g: %s 8888 12\n", argv[0]);
        return 0;
    }

    if (argc == 3) {
        port = static_cast<uint16_t>(atoi(argv[1]));
        threadNum = atoi(argv[2]);
    }

    Scheduler sched;
    InetAddress listenAddr(port);
    CoTcpServer server(&sched, listenAddr, "TCPPingPongServer");
    server.setThreadNum(threadNum);
    server.setConnectionHandler(connHandler);
    server.start();
    sched.wait();
    return 0;
}

