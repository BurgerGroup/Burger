#include "Socket.h"
#include "SocketsOps.h"
#include "IBuffer.h"
#include "CoTcpConnection.h"
#include "burger/base/Util.h"

using namespace burger;
using namespace burger::net;

CoTcpConnection::CoTcpConnection(int sockfd, 
            const InetAddress& localAddr,
            const InetAddress& peerAddr,
            const std::string& connName) 
    : socket_(util::make_unique<Socket>(sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr),
    connName_(connName),
    quit_(false) {
    DEBUG("TcpConnection created [{}] at fd = {}",
         connName_, fmt::ptr(this), sockfd);
    socket_->setKeepAlive(true);
}

CoTcpConnection::~CoTcpConnection() {
    TRACE("TcpConnection {} - {} , {} destroyed [{}] at fd = {}",
        localAddr_.getIpPortStr(), peerAddr_.getIpPortStr(),
             connName_, fmt::ptr(this), socket_->getFd());
}

ssize_t CoTcpConnection::recv(RingBuffer::ptr buf) {
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

ssize_t CoTcpConnection::send(RingBuffer::ptr buf) {
    size_t remain = buf->getReadableBytes();
    return send(buf, remain);
}

ssize_t CoTcpConnection::send(RingBuffer::ptr buf, size_t sendSize) {
    if(quit_) return 0;
    if(sendSize <= 0) return 0;
    while(sendSize) {
        ssize_t n = sockets::write(socket_->getFd(), buf->peek(), sendSize);
        TRACE("send {} bytes ...", n);
        if(n > 0) {
            buf->retrieve(n);
            sendSize -= n;
        }
    } 
    return 0;
}

ssize_t CoTcpConnection::send(const std::string& msg) {
    if(quit_) return 0;
    size_t sendSize = msg.size();
    const char* start = msg.c_str();
    while(sendSize) {
        ssize_t n = sockets::write(socket_->getFd(), start, sendSize);
        TRACE("send {} bytes ...", n);
        if(n > 0) {
            start += n;
            sendSize -= n;
        }
    } 
    return 0;
}




