#include "Connector.h"
#include "EventLoop.h"
#include "Channel.h"
#include <cassert>

using namespace burger;
using namespace burger::net;

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr):
        loop_(loop),
        serverAddr_(serverAddr),
        connect_(false),
        status_(Status::kDisconnected),
        retryDelayMs_(kInitRetryDelayMs) {
    DEBUG("ctor[{}]", fmt::ptr(this));
}

Connector::~Connector() {
    DEBUG("dtor[{}]", fmt::ptr(this));
    assert(!channel_);
}

// 可以跨线程调用
void Connector::start() {
    connect_ = true;
    loop_->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::restart() {
    loop_->assertInLoopThread();
    setStatus(Status::kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

void Connector::stop() {
    connect_ = false;
    loop_->queueInLoop(std::bind(&Connector::stopInLoop, this));
}

const std::string Connector::statusToStr() const {
    switch(status_) {
#define XX(name) \
    case Status::name: \
        return #name;  
    
    XX(kDisconnected);
    XX(kConnecting);
    XX(kConnected);
#undef XX
    default:
        return "unknown status";
    }
}

void Connector::startInLoop() {
    loop_->assertInLoopThread();
    assert(status_ == Status::kDisconnected);
    if(connect_) {
        connect();
    } else {
        // 可能另一个线程stop，将connect_置为false
        DEBUG("do not connect");
    }
}

void Connector::stopInLoop() {
    loop_->assertInLoopThread();
    if(status_ == Status::kConnecting) {
        setStatus(Status::kDisconnected);
        int sockfd = removeAndResetChannel();
        sockets::close(sockfd);
        DEBUG("STOP CONNECT");
    }
}

void Connector::connect() {
    int sockfd = sockets::createNonblockingOrDie();
    int ret = sockets::connect(sockfd, serverAddr_.getSockAddr());

    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno) {
        case 0:
        case EINPROGRESS:  // 非阻塞套接字，未连接成功返回码是EINPROGRESS表示正在连接
        case EINTR:
        case EISCONN:   // 连接成功
            connecting(sockfd);
        break;

        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(sockfd);
        break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            ERROR("connect error in Connector::startInLoop {}", savedErrno);
            sockets::close(sockfd);
        break;

        default:
            ERROR("Unexpected error in Connector::startInLoop {}", savedErrno);
            sockets::close(sockfd);
        // connectErrorCallback_();
        break;
    }
}

void Connector::connecting(int sockfd) {
    setStatus(Status::kConnecting);
    assert(!channel_);
    // channel_并没有在构造函数中初始化，要发生连接才去关注
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback(std::bind(&Connector::handleWrite, this)); // FIXME: unsafe
    channel_->setErrorCallback(std::bind(&Connector::handleError, this)); // FIXME: unsafe
    // channel_->tie(shared_from_this()); is not working,
    // as channel_ is not managed by shared_ptr
    channel_->enableWriting();
}

void Connector::handleWrite() {
    TRACE("Connector::handleWrite {}", status_);
    if (status_ == Status::kConnecting) {
        // todo : why 将channel从EPOLL中移除并置空
        int sockfd = removeAndResetChannel();
        // sockfd 可写并不意味着连接一定成功，还需要getsockopt(sockfd, SOL_SOCKET, SO_ERROR, ...) 再次确认是否发生错误
        int err = sockets::getSocketError(sockfd);
        if (err) {
            WARN("Connector::handleWrite - SO_ERROR =  {} -- {}", err, util::strerror_tl(err));
            retry(sockfd);
        } else if (sockets::isSelfConnect(sockfd)) {
            WARN("Connector::handleWrite - Self connect");
            retry(sockfd);
        } else {
            setStatus(Status::kConnected);
            if (connect_) {
                newConnectionCallback_(sockfd);
            } else {
                sockets::close(sockfd);
            }
        }
    } else {
        // what happened?
        assert(status_ == Status::kDisconnected);
    }
}

void Connector::handleError() {
    ERROR("Connector::handleError state= {}", status_);
    if (status_ == Status::kConnecting) {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        TRACE("SO_ERROR = {} -- {}", err, util::strerror_tl(err));
        retry(sockfd);
    }
}

// 采用back-off策略重连，即重连的时间逐渐延长，0.5s, 1s, 2s, ... 30s
void Connector::retry(int sockfd) {
    sockets::close(sockfd);
    setStatus(Status::kDisconnected);
    if (connect_) {
        INFO("Connector::retry - Retry connecting to {} in {} milliseconds", 
                        serverAddr_.getIpPortStr(), retryDelayMs_);
        // 注册一个定时器重连 
        loop_->runAfter(retryDelayMs_/1000.0,
                        std::bind(&Connector::startInLoop, shared_from_this()));
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);  
    } else {
        DEBUG("do not connect");
    }
}

int Connector::removeAndResetChannel() {
    channel_->disableAll();
    channel_->remove();
    int sockfd = channel_->getFd();
    // Can't reset channel_ here, because we are inside Channel::handleEvent
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this)); // FIXME: unsafe
    return sockfd;
}

void Connector::resetChannel() {
    channel_.reset();
}






