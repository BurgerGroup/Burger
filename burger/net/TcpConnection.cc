#include "TcpConnection.h"
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
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    DEBUG("TcpConnection::ctor[ {} ] at {} fd = {} ", connName_, fmt::ptr(this), sockfd);
    socket_->setKeepAlive(true);  
}

TcpConnection::~TcpConnection() {
    DEBUG("TcpConnection::dtor[ {} ] at {} fd = {} status = {} ", 
            connName_, fmt::ptr(this), channel_->getFd(), statusToStr());
}

bool TcpConnection::getTcpInfo(struct tcp_info& tcpi) const {
    return socket_->getTcpinfo(tcpi);
}

std::string TcpConnection::getTcpInfoString() const {
    return socket_->getTcpInfoString();
}
// 线程安全，可以跨线程调用
void TcpConnection::send(const std::string& message) {
    if(status_ == Status::kConnected) {
        if(loop_->isInLoopThread()) {
            sendInLoop(message);
        }
    }
}



void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(status_ == Status::kConnecting);
    setStatus(Status::kConnected);
    TRACE("[3] usecount = {}", shared_from_this().use_count());
    channel_->tie(shared_from_this());
    // tcpConnection 所对应通道加入Epoll关注
    channel_->enableReading();
    connectionCallback_(shared_from_this());
    TRACE("[4] usecount = {}", shared_from_this().use_count());
}

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    if(status_ == Status::kConnected) {
        setStatus(Status::kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

const std::string TcpConnection::statusToStr() const {
    switch(status_) {
#define XX(name) \
    case Status::name: \
        return #name;  
    
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
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->getFd(), &savedErrno);
    if(n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_, n);
    } else if(n == 0) {
        handleClose();
    } else {
        errno = savedErrno;
        ERROR("TcpConnection::handleRead");
        handleError();
    }
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    TRACE("fd = {} status = {}", channel_->getFd(), statusToStr());
    assert(status_ == Status::kConnected || status_ == Status::kDisconnecting);
    setStatus(Status::kDisconnected);
    channel_->disableAll();
    // 这里给use_count + 1
    TcpConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);     // 这一行可以不调用
    TRACE("[7] usecount = {}", guardThis.use_count());
    closeCallback_(guardThis);   // 调用TcpServer::removeConnection
    TRACE("[11] usecount = {}", guardThis.use_count());
}

void TcpConnection::handleError() {
    int err = sockets::getSocketError(channel_->getFd());
    // todo : wrap strerror_r
    ERROR("TcpConnection::handleError [{}] - SO_ERROR = {} : {}", connName_, err, strerror(err));
}

// TODO 
// 如果数据不能一次发完，则打开channel的写事件开关，分开几次发。
void TcpConnection::sendInLoop(const std::string& message) {
    loop_->assertInLoopThread();
    ssize_t nwrote = 0; 
    size_t remaining = message.size();
    bool faultError = false;
    if(status_ == Status::kDisconnected) { 
        WARN("Disconnected, give up writing!");
        return;
    }
    // ???? todo 如果当前channel没有写事件发生，或者发送buffer已经清空，那么就不通过缓冲区直接发送数据
    if(!channel_->isWriting() && outputBuffer_.getReadableBytes()) {
        nwrote = sockets::write(channel_->getFd(), message);
        if(nwrote >= 0) {
            remaining -= nwrote;
            if(remaining == 0 && writeCompleteCallback_) {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        } else {
            nwrote = 0;
            // TODO 理一下错误处理
            if(errno != EWOULDBLOCK) {
                ERROR("TcpConnection::sendInLoop");
                if(errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

}
