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
    connName_(connName) {
    TRACE("TcpConnection {} - {} , {} created [{}] at fd = {}",
        localAddr_.getIpPortStr(), peerAddr_.getIpPortStr(), connName_, fmt::ptr(this), sockfd);
    socket_->setKeepAlive(true);
}

CoTcpConnection::~CoTcpConnection() {
    TRACE("TcpConnection {} - {} , {} destroyed [{}] at fd = {}",
        localAddr_.getIpPortStr(), peerAddr_.getIpPortStr(),
             connName_, fmt::ptr(this), socket_->getFd());
}

ssize_t CoTcpConnection::recv(RingBuffer::ptr buf) {
    int savedErrno = 0;
    ssize_t n = buf->readFd(socket_->getFd(), savedErrno);
    // todo: if n > 0 == 0 < 0 处理
    return n;
}

ssize_t CoTcpConnection::send(RingBuffer::ptr buf) {
    size_t remain = buf->getReadableBytes();
    while(remain) {
        ssize_t n = sockets::write(socket_->getFd(), buf->peek(), remain);
        if(n > 0) {
            buf->retrieve(n);
            remain -= n;
        }
    } 
    return 0;
}

ssize_t CoTcpConnection::send(RingBuffer::ptr buf, size_t sendSize) {
    if(sendSize <= 0) return 0;
    while(sendSize) {
        ssize_t n = sockets::write(socket_->getFd(), buf->peek(), sendSize);
        if(n > 0) {
            buf->retrieve(n);
            sendSize -= n;
        }
    } 
    return 0;
}
// ssize_t CoTcpConnection::send(const std::string& msg) {
//     return sockets::write(socket_->getFd(), msg);
// }




