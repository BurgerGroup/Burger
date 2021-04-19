#include "TcpServer.h"
#include <cassert>
#include "InetAddress.h"
#include "burger/base/Util.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"

using namespace burger;
using namespace burger::net;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, 
                            const std::string& hostName, bool reuseport):
    loop_(loop),
    hostIpPort_(listenAddr.getIpPortStr()),
    hostName_(hostName),
    acceptor_(util::make_unique<Acceptor>(loop, listenAddr, reuseport)),
    threadPool_(util::make_unique<EventLoopThreadPool>(loop)), // base loop
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback),
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

// todo
void TcpServer::setThreadNum(int numThreads) {
    assert(0 <= numThreads);
    threadPool_->setThreadNum(numThreads);
}

// 多次调用无害，可跨线程调用
void TcpServer::start() {
    if(started_.getAndSet(1) == 0) {
        threadPool_->start(threadInitCallback_);
        assert(!acceptor_->isListening());
        // TODO : ref正确
        loop_->runInLoop(std::bind(&Acceptor::listen, std::ref(acceptor_)));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    loop_->assertInLoopThread();
    // 按照轮叫的方式选择一个EventLoop
    EventLoop* ioLoop = threadPool_->getNextLoop();
    std::string connName = hostName_ + "-" + hostIpPort_ + "#" + std::to_string(nextConnId_);
    ++nextConnId_;
    INFO("TcpServer::newConnection [{}] - new connection [{}] from {} ", 
                hostName_, connName, peerAddr.getIpPortStr());
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    // FIXME poll with zero timeout to double confirm the new connection
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(ioLoop, 
                                    connName, sockfd, localAddr, peerAddr);
    // TRACE("[1] usecount = {}", conn.use_count());  // 1
    connectionsMap_[connName] = conn;
    // TRACE("[2] usecount = {}", conn.use_count());  // 2
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn)); 
    // TRACE("[5] usecount = {}", conn.use_count());  // 2， conn是个临时对象，跳出这个函数又为1--只有列表对象存有
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    loop_->assertInLoopThread();
    INFO("TcpServer::removeConnectionInLoop [{}] - connection {}", hostName_, conn->getName());
    // TRACE("[8] usecount = {}", conn.use_count());  // 3  
    size_t n = connectionsMap_.erase(conn->getName());
    // TRACE("[9] usecount = {}", conn.use_count());  // 2
    assert(n == 1);

    EventLoop* ioLoop = conn->getLoop();    // conn的销毁需要它所在的io线程去操作
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));  //将conn生命周期延长到connectDestroyed
    // TRACE("[10] usecount = {}", conn.use_count());  // 3 --  queueInLoop导致使bind的move到pendingFunctors里引用数+1
}


