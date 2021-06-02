#include "Socket.h"
#include "SocketsOps.h"
#include "Processor.h"
#include "CoTcpConnection.h"
#include "burger/base/Util.h"

using namespace burger;
using namespace burger::net;

CoTcpConnection::CoTcpConnection(Processor* proc,
            int sockfd, 
            const InetAddress& localAddr,
            const InetAddress& peerAddr,
            const std::string& connName) 
    : proc_(proc),
    socket_(util::make_unique<Socket>(sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr),
    connName_(connName),
    quit_(false) {
    TRACE("TcpConnection created [{}] at fd = {}",
         connName_, fmt::ptr(this), sockfd);
    socket_->setKeepAlive(true);
}

CoTcpConnection::~CoTcpConnection() {
    quit_ = true;
    TRACE("TcpConnection {} - {} , {} destroyed [{}] at fd = {}",
        localAddr_.getIpPortStr(), peerAddr_.getIpPortStr(),
             connName_, fmt::ptr(this), socket_->getFd());
}

ssize_t CoTcpConnection::recv(IBuffer::ptr buf) {
    if(quit_) return 0;
    int savedErrno = 0;
    ssize_t n = buf->readFd(socket_->getFd(), savedErrno);
    // todo: if n > 0 == 0 < 0 处理
    return n;
}

void CoTcpConnection::shutdown() {
    quit_ = true;
    socket_->shutdownWrite();
    DEBUG("CoTcpConn {} is shut down.", connName_);
}

void CoTcpConnection::send(IBuffer::ptr buf) {
    send(buf, buf->getReadableBytes());
}


void CoTcpConnection::send(IBuffer* buf) {
    if(proc_->isInProcThread()) {
        sendInProc(std::move(buf->retrieveAllAsString()));
    } else {
        void (CoTcpConnection::*fp)(const std::string& message) = &CoTcpConnection::sendInProc;
        proc_->addTask(std::bind(fp, this, std::move(buf->retrieveAllAsString())));
    }
}

void CoTcpConnection::send(IBuffer::ptr buf, size_t sendSize) {
    if(proc_->isInProcThread()) {
        sendInProc(buf, sendSize);
    } else {
        void (CoTcpConnection::*fp)(IBuffer::ptr buf, size_t sendSize) = &CoTcpConnection::sendInProc;
        proc_->addTask(std::bind(fp, this, buf, sendSize));
    }
}

void CoTcpConnection::sendInProc(IBuffer::ptr buf, size_t sendSize) {
    sendInProc(buf->peek(), sendSize);
    buf->retrieve(sendSize);
}

void CoTcpConnection::send(const std::string& msg) {
    if(proc_->isInProcThread()) {
        sendInProc(msg);
    } else {
        void (CoTcpConnection::*fp)(const std::string& message) = &CoTcpConnection::sendInProc;
        proc_->addTask(std::bind(fp, this, std::move(msg)));
    }
}

void CoTcpConnection::send(const char* start, size_t sendSize) {
    send(std::string(start, sendSize));
}

void CoTcpConnection::sendInProc(const std::string& msg) {
    sendInProc(msg.c_str(), msg.size());
}

void CoTcpConnection::sendInProc(const char* start, size_t sendSize) {
    if(quit_) {
        WARN("Disconnected, give up writing");
        return;
    }
    ssize_t sendBytes = 0;
    while(sendSize) {
        ssize_t nwrote = sockets::write(socket_->getFd(), start, sendSize);
        DEBUG("send {} bytes ...", nwrote); 
        if(nwrote >= 0) {
            sendBytes += nwrote;   
            start += nwrote;
            sendSize -= nwrote;
        } else {  // nwrote < 0
            if(errno != EWOULDBLOCK) {
                ERROR("CoTcpConnection can't send errno = {}", errno);
                // 连接已经关闭导致
                // socket write中，对方socket中断，发送端write会先返回已经发送的字节数,再次write时返回-1,errno号为ECONNRESET
                // write一个已经接受到RST的socket，系统内核会发送SIGPIPE给发送进程，如果进程catch/ignore这个信号，write都返回EPIPE错误
                if(errno == EPIPE || errno == ECONNRESET) {
                    WARN("peer is disconnected..");
                }
                // 本端关闭，对端关闭都要注意quit_的改变
                quit_ = true;
                break;
            }
        }
    } 
}

void CoTcpConnection::setTcpNoDelay(bool on) {
    socket_->setTcpNoDelay(on);
}




 