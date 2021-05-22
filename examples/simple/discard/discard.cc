#include "discard.h"
#include "burger/net/RingBuffer.h"
using namespace std::placeholders;

DiscardServer::DiscardServer(Scheduler* sched, const InetAddress& listenAddr)
    : server_(sched, listenAddr) {
    server_.setConnectionHandler(std::bind(&DiscardServer::connHandler, this, _1));
}

void DiscardServer::start() {
    server_.start();
}

void DiscardServer::connHandler(const CoTcpConnection::ptr& conn) {
    RingBuffer::ptr buffer = std::make_shared<RingBuffer>();
    while(conn->recv(buffer) > 0) {
        Timestamp ts = Timestamp::now();
        std::string msg(buffer->retrieveAllAsString());
        INFO("{} discards {} bytes received at {}", 
            conn->getName(), msg.size(), ts.toFormatTime());
    }
}