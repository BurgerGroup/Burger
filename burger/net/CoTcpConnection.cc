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
    size_t sendSize = buf->getReadableBytes();
    ssize_t nSend = send(buf->peek(), sendSize);
    buf->retrieve(sendSize);
    return nSend;
}

ssize_t CoTcpConnection::send(RingBuffer::ptr buf, size_t sendSize) {
    ssize_t nSend = send(buf->peek(), sendSize);
    buf->retrieve(sendSize);
    return nSend;
}

ssize_t CoTcpConnection::send(const std::string& msg) {
    return send(msg.c_str(), msg.size());
}

ssize_t CoTcpConnection::send(const char* start, size_t sendSize) {
    if(quit_) {
        WARN("Disconnected, give up writing");
        return 0;
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
                ERROR("CoTcpConnection can't send");
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
    return sendBytes;

}



void CoTcpConnection::setTcpNoDelay(bool on) {
    socket_->setTcpNoDelay(on);
}




