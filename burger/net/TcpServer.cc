#include "TcpServer.h"
#include <cassert>
#include "InetAddress.h"
#include "burger/base/Util.h"
#include "Acceptor.h"
#include "EventLoop.h"

using namespace burger;
using namespace burger::net;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, 
                                std::string hostName, bool reuseport):
    loop_(loop),
    hostIpPort_(listenAddr.getIpPortStr()),
    hostName_(hostName),
    acceptor_(util::make_unique<Acceptor>(loop, listenAddr, reuseport)),
    //...
    nextConnId_(1) {
    assert(loop);
    acceptor_->setNewConnectionCallback(
            std::bind(&TcpServer::newConnection, this, 
            std::placeholders::_1,   // socket fd 
            std::placeholders::_2));   // peerAddr
}

TcpServer::~TcpServer() {
    loop_->assertInLoopThread();
    TRACE("TcpServer::~TcpServer [ {} ] destructing", hostName_);
}


// 多次调用无害，可跨线程调用
void TcpServer::start() {
    if(started_.getAndSet(1) == 0) {
        // threadpool
        assert(!acceptor_->isListening());
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_));
    }

}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    loop_->assertInLoopThread();
    std::string connName = hostName_ + "-" + hostIpPort_ + "#" + std::to_string(nextConnId_);
    ++nextConnId_;
    INFO("TcpServer::newConnection [{}] - new connection [{}] from {} ", 
                hostName_, connName, peerAddr.getIpPortStr());
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    // FIXME poll with zero timeout to double confirm the new connection
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(loop_, 
                                    connName, sockfd, localAddr, peerAddr);
    connectionsMap_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    
    conn->connectEstablished();
}


