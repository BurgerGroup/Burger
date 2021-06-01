#include "TcpConnection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"

using namespace burger;
using namespace burger::net;

void burger::net::defaultConnectionCallback(const TcpConnectionPtr& conn) {
    TRACE("{} -> {} is {}", conn->getLocalAddress().getIpPortStr(), 
            conn->getPeerAddress().getIpPortStr(), (conn->isConnected() ? "UP" : "DOWN"));
    // do not call conn->forceClose(), because some users want to register message callback only.
}

void burger::net::defaultMessageCallback(const TcpConnectionPtr&,
                                        IBuffer& buf,
                                        Timestamp) {
    buf.retrieveAll();
}


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
    peerAddr_(peerAddr),
    highWaterMark_(64*1024*1024) {  // 64 M
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, 
                                    this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
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
        } else {
            // bind other object's private member function 
            void (TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp, this, std::move(message)));  // 这里message跨线程的话，异步只能复制一次过去
            // loop_->runInLoop(std::bind(fp, this, message));
        }
    }
}

// void TcpConnection::send(Buffer& buf) {
//     if(status_ == Status::kConnected) {
//         if(loop_->isInLoopThread()) {
//             sendInLoop(buf.peek(), buf.getReadableBytes());
//             buf.retrieveAll();
//         } else {
//             void (TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
//             loop_->runInLoop(std::bind(fp, this, std::move(buf.retrieveAllAsString())));
//         }
//     }
// }

void TcpConnection::send(IBuffer& buf) {
    if(status_ == Status::kConnected) {
        const std::type_info& info = typeid(buf);
        if(loop_->isInLoopThread()) {
            if(info == typeid(Buffer)) {
                sendInLoop(buf.peek(), buf.getReadableBytes());
                buf.retrieveAll();
            }
            else if(info == typeid(RingBuffer)) {
                // FIXME : send 2 parts separately by pointer
                sendInLoop(std::move(buf.retrieveAllAsString()));
            }
        } else {
            if(info == typeid(Buffer)) {
                void (TcpConnection::*fp)(const void* data, size_t len) = &TcpConnection::sendInLoop;
                loop_->runInLoop(std::bind(fp, this, buf.peek(), buf.getReadableBytes()));
                buf.retrieveAll();
            }
            else if(info == typeid(RingBuffer)) {
                void (TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
                // FIXME : send 2 parts separately by pointer
                loop_->runInLoop(std::bind(fp, this, std::move(buf.retrieveAllAsString())));
            }
        }
    }
}

// 应用程序想关闭连接，但是可能正处于发送数据的过程中，output buffer 中有数据还没发完，不能直接调用close
void TcpConnection::shutdown() {
    // FIXME: use 原子性操作compare and swap
    if (status_ == Status::kConnected) {
        // 如果正在写，他只是把状态改变，但再shutdownInLoop中并没有去关闭连接
        setStatus(Status::kDisconnecting);
        // FIXME: shared_from_this()?
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::forceClose() {
    if(status_ == Status::kConnected || status_ == Status::kDisconnecting) {
        setStatus(Status::kDisconnecting);
        loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::setTcpNoDelay(bool on)  {
    socket_->setTcpNoDelay(on);
}

void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(status_ == Status::kConnecting);
    setStatus(Status::kConnected);
    // TRACE("[3] usecount = {}", shared_from_this().use_count());  // shared_from_this + 临时 -- 3 
    channel_->tie(shared_from_this());  // tie是weak_ptr, 计数不变
    channel_->enableReading();     // tcpConnection 所对应通道加入Epoll关注
    connectionCallback_(shared_from_this());
    // TRACE("[4] usecount = {}", shared_from_this().use_count());   // 3，临时，下一个马上变2
}

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    if(status_ == Status::kConnected) {
        // 和handleclose重复，因为在某些情况下可以不经handleClose而直接调用connectDestriyed
        setStatus(Status::kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this()); // 回调用户的回调函数
    }
    channel_->remove();
    TRACE("channel of conn {} is removed.", connName_);
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
    ssize_t n = inputBuffer_.readFd(channel_->getFd(), savedErrno);
    if(n > 0) {
        messageCallback_(shared_from_this(), inputBuffer_, receiveTime);
    } else if(n == 0) {
        handleClose();
    } else {
        errno = savedErrno;
        ERROR("TcpConnection::handleRead");
        handleError();
    }
}

void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    // 如果还在关注EPOLLOUT事件，说明之前的数据还没发送给完成，则将缓冲区的数据发送
    if (channel_->isWriting()) {
        ssize_t n = sockets::write(channel_->getFd(),
                                outputBuffer_.peek(),
                                outputBuffer_.getReadableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.getReadableBytes() == 0) {  // 说明已经发送完成，缓冲区已清空
                channel_->disableWriting();   // 停止关注POLLOUT事件，以免出现busy-loop
                if (writeCompleteCallback_) {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (status_ == Status::kDisconnecting) {  // 如果之前调用了shutdown,但是还在output，没用关闭读端，留到此时关闭
                    shutdownInLoop();
                }
            }
        } else { // n <= 0
            ERROR("TcpConnection::handleWrite");
        }
    } else {  // 没有关注EPOLLOUT事件
        TRACE("Connection fd = {} is down, no more writing", channel_->getFd());
    }
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    TRACE("fd = {} status = {}", channel_->getFd(), statusToStr());
    assert(status_ == Status::kConnected || status_ == Status::kDisconnecting);
    // we don't close fd, leave it to dtor, se we can find leaks easily
    setStatus(Status::kDisconnected);
    channel_->disableAll();
    TcpConnectionPtr guardThis(shared_from_this());   // 3 
    connectionCallback_(guardThis);     // 回调了用户的连接到来的回调函数， 这里和connectDestroyed重复,reason in connectDestroyed
    // TRACE("[7] usecount = {}", guardThis.use_count());  // 3 -- connectionMap_, tie_提升， guardThis
    closeCallback_(guardThis);   // TcpServer::newconnection 注册的 TcpServer::removeConnection
    // TRACE("[11] usecount = {}", guardThis.use_count());   // 3
}

void TcpConnection::handleError() {
    int err = sockets::getSocketError(channel_->getFd());
    // todo : wrap strerror_r
    ERROR("TcpConnection::handleError [{}] - SO_ERROR = {} : {}", connName_, err, strerror(err));
}


// 如果数据不能一次发完，则打开channel的写事件开关，分开几次发。
void TcpConnection::sendInLoop(const std::string& message) {
    sendInLoop(message.c_str(), message.size());
}

// 此接口也方便了send(Buffer&)
void TcpConnection::sendInLoop(const void* data, size_t len) {
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if(status_ == Status::kDisconnected){
        WARN("disconnected, give up writing");
        return;
    }
    // 通道没关注可写事件，并且发送缓冲区没数据，直接write
    if (!channel_->isWriting() && outputBuffer_.getReadableBytes() == 0) {
        nwrote = sockets::write(channel_->getFd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            // 写完
            if (remaining == 0 && writeCompleteCallback_) {
                // 为什么我们的writeCompleteCallback_ 都要queueInLoop而不是 runInLoop
                // run和queue的区别在于，在IO线程中调用run这个函数会立刻执行，而调用queue的话，会等下次从poll返回之后执行doPend的时候才执行
                // 一种特殊情况，writeCompleteCallback中向所在的socket写入1byte的信息，每次进行到这马上有趣发送1 bytes,然后又触发writeComplete,然后进入递归，爆栈
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        } else { // nwrote < 0
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                ERROR("TcpConnection::sendInLoop");
                if (errno == EPIPE || errno == ECONNRESET) {// FIXME: any others?
                    faultError = true;
                }
            }
        }
    }
    assert(remaining <= len);
    // 没有错误，且还有没有写完的数据(说明内核发送缓冲区满，要将未写完的数据添加到output buffer中)
    if (!faultError && remaining > 0) {
        size_t oldLen = outputBuffer_.getReadableBytes();  // outputbuf 本身有的数据量
        // 如果超过highWaterMark_(高水位标)，回调highWaterMarkCallback_
        if (oldLen + remaining >= highWaterMark_
            && oldLen < highWaterMark_
            && highWaterMarkCallback_) {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append(static_cast<const char*>(data)+nwrote, remaining);
        // 如果没有关注EPOLLOUT事件则关注
        if(!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

// 当服务器端主动断开与客户端的连接，这意味着客户端read返回0, close(connfd),
// 服务器端收到的事件为 POLLHUP | POLLIN
void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if(!channel_->isWriting()) {  
        socket_->shutdownWrite();  
    }
}

void TcpConnection::forceCloseInLoop() {
    loop_->assertInLoopThread();
    if(status_ == Status::kConnected || status_ == Status::kDisconnecting) {
        handleClose();
    }
}
