#include "burger/net/CoTcpServer.h"
#include "burger/base/Log.h"
#include "burger/net/RingBuffer.h"
#include "burger/net/Scheduler.h"

using namespace burger;
using namespace burger::net;

void connHandler(CoTcpConnection::ptr conn) {
    conn->setTcpNoDelay(true);
    RingBuffer::ptr buf = std::make_shared<RingBuffer>();
    while(conn->recv(buf) > 0) {
        // // printf("++++++++++++++ReadableBytes: %lu\n", buf.getReadableBytes());
        // buf->retrieve(50);
        // // printf("++++++++++++++WritableBytes: %lu\n", buf.getWritableBytes());
        // // printf("++++++++++++++ReadableBytes: %lu\n", buf.getReadableBytes());
        // buf->append(std::string(42, 's'));
        // // printf("++++++++++++++WritableBytes: %lu\n", buf.getWritableBytes());
        // std::string tmp(8, 's');
        // buf->prepend(reinterpret_cast<const void*>(tmp.c_str()), 8); // 把前面填满
        conn->send(buf);
    }
}

int main(int argc, char* argv[]) {
    LOG_LEVEL_ERROR;
    if (argc < 4) {
        fprintf(stderr, "Usage: server <address> <port> <threads>\n");
    } else {

        LOGGER(); LOG_LEVEL_WARN;

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

