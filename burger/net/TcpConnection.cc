#include "TcpConnection.h"
#include "InetAddress.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"

using namespace burger;
using namespace burger::net;

TcpConnection::TcpConnection(EventLoop* loop, 
                const std::string& connName,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr):
    loop_(loop),
    connName_(connName),
    status_(Status::kConnecting),
    socket_(util::make_unique<Socket>(sockfd)), 
    channel_(util::make_unique<Channel>(loop, sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr) {
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, 
                                    this, std::placeholders::_1));
    DEBUG("TcpConnection::ctor[ {} ] at {} fd = {} ", connName_, fmt::ptr(this), sockfd);
    socket_->setKeepAlive(true);  
}

TcpConnection::~TcpConnection() {
    DEBUG("TcpConnection::dtor[ {} ] at {} fd = {} status = {} ", 
            connName_, fmt::ptr(this), channel_->getFd(), statusToStr());
}

void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(status_ == Status::kConnecting);
    setStatus(Status::kConnected);
    channel_->tie(shared_from_this());
    // tcpConnection 所对应通道加入Epoll关注
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

const std::string TcpConnection::statusToStr() const {
    switch(status_) {
#define XX(name) \
    case Status::name: \
        return #name; \ 
    
    XX(kDisconnected);
    XX(kConnecting);
    XX(kConnected);
    XX(kDisconnecting);
#undef XX
    default:
        return "unknown status";
    }
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    loop_->assertInLoopThread();
    char buf[65536];
    ssize_t n = ::read(channel_->getFd(), buf, sizeof(buf));
    messageCallback_(shared_from_this(), buf, n);
}
