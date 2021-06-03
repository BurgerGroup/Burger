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
    nextConnId_(1),
    idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    assert(idleFd_ > 0);
    listenSock_->setReuseAddr(true);
    listenSock_->bindAddress(listenAddr_);
    DEBUG("CoTcpServer created {}", hostName_);
}

CoTcpServer::~CoTcpServer() {
    sockets::close(idleFd_);
    DEBUG("CoTcpServer destroyed {}", hostName_);
}

void CoTcpServer::setThreadNum(size_t threadNum) {
    assert(threadNum > 0);
    sched_->setWorkProcNum(threadNum);
}

// 多次调用无害
void CoTcpServer::start() {
    if(started_.getAndSet(1) == 0) {
        sched_->startAsync();
        listenSock_->listen();
        sched_->addMainTask(std::bind(&CoTcpServer::startAccept, this));
    }
}

void CoTcpServer::setConnectionHandler(const ConnectionHandler& handler) {
    connHandler_ = handler;
}

void CoTcpServer::startAccept() {
    while(started_.get()) {
        InetAddress peerAddr;
        int connfd = listenSock_->accept(peerAddr);
        if(connfd > 0) {
            // TRACE("Accept of {}", peerAddr.getIpPortStr());
            std::string connName(std::move(hostName_ + "-" + hostIpPort_ + "#" + std::to_string(nextConnId_++)));
            // 将conn交给一个sub processor
            Processor* proc = sched_->pickOneWorkProcessor();
            CoTcpConnection::ptr conn = std::make_shared<CoTcpConnection>(proc, connfd,
                        listenAddr_, peerAddr, std::move(connName));
            // conn->setConnEstablishCallback(connEstablishCallback_);
            proc->addToConnMap(connfd, conn);
            // 此处跨线程调用
            proc->addTask(std::bind(connHandler_, conn));
        } else {
            ERROR("accept error");
            // 当fd达到上限，先占住一个空的fd,然后当fd满了，就用这个接受然后关闭
            // 这样避免一直水平电平一直触发，先去读走他
            if(errno == EMFILE) {   
                ERROR("Exceed the maximum fd nums");
                sockets::close(idleFd_);
                idleFd_ = ::accept(listenSock_->getFd(), nullptr, nullptr);
                sockets::close(idleFd_);
                idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
            }
        }
    }
}






