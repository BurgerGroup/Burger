#include "burger/net/CoTcpServer.h"
#include "burger/base/Log.h"
#include "burger/net/Buffer.h"
#include "burger/net/Scheduler.h"

using namespace burger;
using namespace burger::net;

void connHandler(CoTcpConnection::ptr conn) {
    conn->setTcpNoDelay(true);
    Buffer::ptr buf = std::make_shared<Buffer>();
    while(conn->recv(buf) > 0) {
        conn->send(buf);
    }
}

int main(int argc, char* argv[]) {
    // LOGGER(); LOG_LEVEL_TRACE;
    if (argc < 4) {
        fprintf(stderr, "Usage: server <address> <port> <threads>\n");
    } else {
        LOGGER("./pingpong_coServer.log", "pingpong coServer"); LOG_LEVEL_ERROR;

        const char* ip = argv[1];
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress listenAddr(ip, port);
        int threadNum = atoi(argv[3]);

        Scheduler sched;
        CoTcpServer server(&sched, listenAddr, "TCPPingPongServer");
        server.setThreadNum(threadNum);
        server.setConnectionHandler(connHandler);
        server.start();
        sched.wait();
    }
}

