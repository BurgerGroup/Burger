#include "discard.h"

using namespace std::placeholders;

DiscardServer::DiscardServer(const InetAddress& listenAddr, int threadNum, const std::string& name)
    : server_(listenAddr, threadNum, name) {
    server_.setConnectionHandler(std::bind(&DiscardServer::connHandler, this, _1));
}
void DiscardServer::start() {
    server_.start();
}

void DiscardServer::connHandler(CoTcpConnection::ptr conn) {
    RingBuffer::ptr buffer = std::make_shared<RingBuffer>();
    while(conn->recv(buffer) > 0) {
        Timestamp ts = Timestamp::now();
        std::string msg(buffer->retrieveAllAsString());
        INFO("{} discards {} bytes received at {}", 
            conn->getName(), msg.size(), ts.toFormatTime());
    }
}