#include "CoTcpServer.h"
#include "Scheduler.h"
#include "SocketsOps.h"
#include "Socket.h"
#include "CoTcpConnection.h"
#include "burger/base/Util.h"
using namespace burger;
using namespace burger::net;

namespace {

void defualtHandler(CoTcpConnection::ptr conn) {
    INFO("new connection, peer addr : {}", conn->getPeerAddr().getPortStr());
}

// void CoDefaultConnEstablishCallback(const CoTcpConnectionPtr& conn) {
//     TRACE("{} - {} conn is established", conn->getLocalAddress().getIpPortStr(),
//         conn->getPeerAddr().getIpPortStr());
// }

} // namespace

CoTcpServer::CoTcpServer(uint16_t port, int threadNum, const std::string& name)
    : listenAddr_(port),
    listenSock_(util::make_unique<Socket>(sockets::createNonblockingOrDie())),
    sched_(util::make_unique<Scheduler>(threadNum)),
    connHandler_(defualtHandler),
    hostIpPort_(listenAddr_.getIpPortStr()),
    hostName_(name),
    nextConnId_(1) {
    listenSock_->setReuseAddr(true);
    listenSock_->bindAddress(listenAddr_);
    DEBUG("CoTcpServer created {}", hostName_);
}

CoTcpServer::CoTcpServer(const std::string& ip, uint16_t port, int threadNum, const std::string& name)
    : listenAddr_(ip, port),
    listenSock_(util::make_unique<Socket>(sockets::createNonblockingOrDie())),
    sched_(util::make_unique<Scheduler>(threadNum)),
    connHandler_(defualtHandler),
    hostIpPort_(listenAddr_.getIpPortStr()),
    hostName_(name),
    nextConnId_(1) {
    listenSock_->setReuseAddr(true);
    listenSock_->bindAddress(listenAddr_);
    DEBUG("CoTcpServer created {}", hostName_);
}

CoTcpServer::CoTcpServer(const InetAddress& listenAddr, int threadNum, const std::string& name) 
    : listenAddr_(listenAddr),
    listenSock_(util::make_unique<Socket>(sockets::createNonblockingOrDie())),
    sched_(util::make_unique<Scheduler>(threadNum)),
    connHandler_(defualtHandler),
    hostIpPort_(listenAddr_.getIpPortStr()),
    hostName_(name),
    nextConnId_(1) {
    listenSock_->setReuseAddr(true);
    listenSock_->bindAddress(listenAddr_);
    DEBUG("CoTcpServer created {}", hostName_);
}


CoTcpServer::~CoTcpServer() {
    sched_->wait();  // todo: need put here ? 
    DEBUG("CoTcpServer destroyed {}", hostName_);
}

// 多次调用无害
void CoTcpServer::start() {
    if(started_.getAndSet(1) == 0) {
        sched_->startAsync();
        listenSock_->listen();
        sched_->addTask(std::bind(&CoTcpServer::startAccept, this), "Accept");
    }
}

void CoTcpServer::setConnectionHandler(const ConnectionHandler& handler) {
    connHandler_ = handler;
}

void CoTcpServer::startAccept() {
    while(true) {
        InetAddress peerAddr;
        int connfd = listenSock_->accept(peerAddr);
        if(connfd > 0) {
            TRACE("Accept of {}", peerAddr.getIpPortStr());
            std::string connName = hostName_ + "-" + hostIpPort_ + "#" + std::to_string(nextConnId_++);
            CoTcpConnection::ptr conn = std::make_shared<CoTcpConnection>(connfd, 
                        listenAddr_, peerAddr, connName);
            // conn->setConnEstablishCallback(connEstablishCallback_);
            sched_->addTask(std::bind(connHandler_, conn));
        } 
        // todo : idlefd
    }
}






