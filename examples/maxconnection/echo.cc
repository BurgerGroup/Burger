#include "echo.h"
#include "burger/net/RingBuffer.h"
using namespace std::placeholders;

EchoServer::EchoServer(Scheduler* sched, const InetAddress& listenAddr, int maxConnections)
    : server_(sched, listenAddr, "EchoServer"),
    kMaxConnections_(maxConnections) {
    server_.setConnectionHandler(std::bind(&EchoServer::connHandler, this, _1));
}

void EchoServer::start() {
    server_.start();
}

void EchoServer::connHandler(const CoTcpConnection::ptr& conn) {
    numConnected_.increment();
    INFO("numConnected = {}", numConnected_.get());
    if(static_cast<int>(numConnected_.get()) > kMaxConnections_) {
        INFO("connected too many, shutdown this conn");
        conn->send("too many connections");
        conn->shutdown();
    } else {
        RingBuffer::ptr buffer = std::make_shared<RingBuffer>();
        while(conn->recv(buffer) > 0) {
            INFO("{} echo {} bytes", conn->getName(), buffer->getReadableBytes());
            conn->send(buffer);
        }
    }
    numConnected_.decrement();
    
}



