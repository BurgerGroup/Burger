#include "burger/net/TcpServer.h"
#include "burger/net/IBuffer.h"
#include "burger/base/Log.h"
#include "burger/net/EventLoop.h"
#include "burger/net/InetAddress.h"

#include <utility>
#include <string>
#include <stdio.h>
#include <unistd.h>

using namespace burger;
using namespace burger::net;

void onConnection(const TcpConnectionPtr& conn) {
    if (conn->isConnected()) {
        conn->setTcpNoDelay(true);
    }
}

void onMessage(const TcpConnectionPtr& conn, IBuffer& buf, Timestamp) {
    // printf("++++++++++++++ReadableBytes: %lu\n", buf.getReadableBytes());
    // buf.retrieve(50);
    // // printf("++++++++++++++WritableBytes: %lu\n", buf.getWritableBytes());
    // // printf("++++++++++++++ReadableBytes: %lu\n", buf.getReadableBytes());
    // buf.append(std::string(42, 's'));
    // // printf("++++++++++++++WritableBytes: %lu\n", buf.getWritableBytes());
    // std::string tmp(8, 's');
    // buf.prepend(reinterpret_cast<const void*>(tmp.c_str()), 8); // 把前面填满
    conn->send(buf);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: server <address> <port> <threads>\n");
    }
    else {
        LOGGER("./pingpong_server.log", "pingpong coServer"); LOG_LEVEL_ERROR;

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