#include "TcpServer.h"
#include <cassert>
#include "InetAddress.h"
#include "burger/base/Util.h"
#include "Acceptor.h"
#include "EventLoop.h"

using namespace burger;
using namespace burger::net;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, 
                            const std::string& hostName, bool reuseport):
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
        // TODO : ref正确
        loop_->runInLoop(std::bind(&Acceptor::listen, std::ref(acceptor_)));
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
    TRACE("[1] usecount = {}", conn.use_count());
    connectionsMap_[connName] = conn;
    TRACE("[2] usecount = {}", conn.use_count());
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    conn->connectEstablished();
    TRACE("[5] usecount = {}", conn.use_count());
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    loop_->assertInLoopThread();
    INFO("TcpServer::removeConnectionInLoop [{}] - connection {}", hostName_, conn->getName());
    TRACE("[8] usecount = {}", conn.use_count());
    size_t n = connectionsMap_.erase(conn->getName());
    TRACE("[9] usecount = {}", conn.use_count());    
    assert(n == 1);
    loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    TRACE("[10] usecount = {}", conn.use_count());    
}


