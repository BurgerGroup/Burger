#include "burger/net/CoTcpServer.h"
#include "burger/base/Log.h"
#include "burger/net/RingBuffer.h"
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
    InetAddress listenAddr(8888);
    CoTcpServer server(listenAddr, 2);
    server.start();
    return 0;
}