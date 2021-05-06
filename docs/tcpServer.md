## TcpServer

## 接收新连接

reactor和tensorflow类似，先把图纸给设置好，然后根据不同的事件调用不同的callBack

TcpServer接收新链接是靠Acceptor

```cpp
class Acceptor : boost::noncopyable {
public:
    ...
    void setNewConnectionCallback(const NewConnectionCallback& cb) { newConnectionCallback_ = cb; }
    void listen();
    ...
private: 
    void handleRead();
    ...
    std::unique_ptr<Socket> acceptSocket_;
    std::unique_ptr<Channel> acceptChannel_;

    int idleFd_;   // 占位fd,用于fd满的情况，避免一直电平触发
};

// 构造的时候，就把callBack设置好
acceptChannel_->setReadCallback(std::bind(&Acceptor::handleRead, this));

// listen实际作用的是socket的listen
// 这里只是把accept事件挂上epoll监听
void Acceptor::listen() {
    loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_->listen();
    acceptChannel_->enableReading();
}

// 而他关注的读事件对应的callback是handleRead，也是acceptor的核心
// 一旦新连接到来，可读事件触发，这里去accept连接，并处理一些异常情况
// 然后这里newConnectionCallback_ 就是之前绑定的TcpServer的newConnection
void Acceptor::handleRead() {
    loop_->assertInLoopThread();
    InetAddress peerAddr;
    int connfd = acceptSocket_->accept(peerAddr);
    if(connfd >= 0) {
        TRACE("Accept of {}", peerAddr.getIpPortStr());
        if(newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr);
        } else {
            // 未注册回调函数，则关闭
            sockets::close(connfd);
        }
    } else {
        ERROR("in Acceptor::handleRead");
        // 当fd达到上限，先占住一个空的fd,然后当fd满了，就用这个接受然后关闭
        // 这样避免一直水平电平一直触发，先去读走他
        if(errno == EMFILE) {   
            sockets::close(idleFd_);
            idleFd_ = ::accept(acceptSocket_->getFd(), nullptr, nullptr);
            sockets::close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

```

```cpp
// 在tcpServer构造时就把newConnection设置好
acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, 
        std::placeholders::_1,   // socket fd 
        std::placeholders::_2));   // peerAddr

// acceptor在handleRead的时候调用newConnection
// 将conn存储到connectionsMap_使得他的生命周期持续
// 然后交给一个ioLoop去connectEstablished
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    loop_->assertInLoopThread();
    // 按照轮叫的方式选择一个EventLoop
    EventLoop* ioLoop = threadPool_->getNextLoop();
    std::string connName = hostName_ + "-" + hostIpPort_ + "#" + std::to_string(nextConnId_);
    ++nextConnId_;
    INFO("TcpServer::newConnection [{}] - new connection [{}] from {} ", 
                hostName_, connName, peerAddr.getIpPortStr());
    InetAddress localAddr(sockets::getLocalAddr(sockfd));

    TcpConnectionPtr conn = std::make_shared<TcpConnection>(ioLoop, 
                                    connName, sockfd, localAddr, peerAddr);

    connectionsMap_[connName] = conn;

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn)); 
}
```

```cpp
// 这里实际上是去将conn的添加到epoll关注
// 然后执行用户设置的connectionCallback_
void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(status_ == Status::kConnecting);
    setStatus(Status::kConnected);
    channel_->tie(shared_from_this());  // tie是weak_ptr, 计数不变
    channel_->enableReading();     // tcpConnection 所对应通道加入Epoll关注
    connectionCallback_(shared_from_this());
}
```

