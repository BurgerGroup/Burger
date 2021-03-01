#include "chargen.h"
#include "burger/net/EventLoop.h"

ChargenServer::ChargenServer(EventLoop* loop,
        const InetAddress& listenAddr,
        bool print):
        server_(loop, listenAddr, "ChargenServer"),
        transferred_(0),
        startTime_(Timestamp::now()) {
    server_.setConnectionCallback(std::bind(&ChargenServer::onConnection, this,
                                            std::placeholders::_1));  // conn
    server_.setMessageCallback(std::bind(&ChargenServer::onMessage, this, 
                                            std::placeholders::_1,    // conn 
                                            std::placeholders::_2,    // buffer
                                            std::placeholders::_3));  // timestamp
    server_.setWriteCompleteCallback(std::bind(&ChargenServer::onWriteComplete, this, 
                                            std::placeholders::_1));   // conn
    if (print) {
        loop->runEvery(3.0, std::bind(&ChargenServer::printThroughput, this));
    }

    std::string line;
    for (int i = 33; i < 127; ++i) {
        line.push_back(char(i));
    }
    line += line;
    for (size_t i = 0; i < 127-33; ++i) {
        message_ += line.substr(i, 72) + '\n';
    }
}

void ChargenServer::start() {
    server_.start();
}

void ChargenServer::onConnection(const TcpConnectionPtr& conn) {
    INFO("ChargenServer - {} -> {} is {}", 
        conn->getPeerAddress().getIpPortStr(), 
        conn->getLocalAddress().getIpPortStr(),
        (conn->isConnected() ? "UP" : "DOWN"));
    if(conn->isConnected()) {
        conn->setTcpNoDelay(true);
        conn->send(message_);
    }
}

void ChargenServer::onMessage(const TcpConnectionPtr& conn,
                   Buffer& buf,
                   Timestamp time) {
    std::string msg(buf.retrieveAllAsString());
    INFO("{} discards {} bytes received at {}",
            conn->getName(), msg.size(), time.toFormatTime());
}

void ChargenServer::onWriteComplete(const TcpConnectionPtr& conn) {
    // INFO("CALL WriteComplete");
    transferred_ += message_.size();
    conn->send(message_);
}

void ChargenServer::printThroughput() {
    Timestamp endTime(Timestamp::now());
    double time = timeDifference(endTime, startTime_);
    std::cout << static_cast<double>(transferred_)/time/1024/1024 << " MiB/s" << std::endl;
    transferred_ = 0;
    startTime_ = endTime;
}




