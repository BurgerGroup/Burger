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


CoTcpServer::CoTcpServer(Scheduler* sched, const InetAddress& listenAddr, 
                            const std::string& name, bool reuseport) 
    : sched_(sched),
    listenAddr_(listenAddr),
    listenSock_(util::make_unique<Socket>(sockets::createNonblockingOrDie())),
    connHandler_(defualtHandler),
    hostIpPort_(listenAddr_.getIpPortStr()),
    hostName_(name),
    nextConnId_(1) {
    listenSock_->setReuseAddr(true);
    listenSock_->bindAddress(listenAddr_);
    DEBUG("CoTcpServer created {}", hostName_);
}

CoTcpServer::~CoTcpServer() {
    DEBUG("CoTcpServer destroyed {}", hostName_);
}

void CoTcpServer::setThreadNum(size_t threadNum) {
    assert(threadNum > 0);
    sched_->setThreadNum(threadNum);
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
            // 将conn交给一个sub processor
            Processor* proc = sched_->pickOneWorkProcessor();
            CoTcpConnection::ptr conn = std::make_shared<CoTcpConnection>(proc, connfd,
                        listenAddr_, peerAddr, connName);
            // conn->setConnEstablishCallback(connEstablishCallback_);
            // 此处跨线程调用
            proc->addPendingTask(std::bind(connHandler_, conn), "connHandler");
        } 
        // todo : idlefd
    }
}






