#include "burger/net/TcpServer.h"
#include "burger/net/RingBuffer.h"
#include "burger/base/Log.h"
#include "burger/net/EventLoop.h"
#include "burger/net/InetAddress.h"

#include <utility>

#include <stdio.h>
#include <unistd.h>

using namespace burger;
using namespace burger::net;

void onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        conn->setTcpNoDelay(true);
    }
}

void onMessage(const TcpConnectionPtr& conn, RingBuffer& buf, Timestamp) {
    conn->send(buf);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: server <address> <port> <threads>\n");
    }
    else {
        INFO("pid = {}, tid = {}", getpid(), util::tid());
        LOG_LEVEL_INFO;

        const char* ip = argv[1];
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress listenAddr(ip, port);
        int threadCount = atoi(argv[3]);

        EventLoop loop;

        TcpServer server(&loop, listenAddr, "PingPong");

        server.setConnectionCallback(onConnection);
        server.setMessageCallback(onMessage);

        if (threadCount > 1) {
            server.setThreadNum(threadCount);
        }

        server.start();

        loop.loop();
    }
}