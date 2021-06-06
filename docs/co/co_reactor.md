
## CoTcpServer和传统Reactor TcpServer的比较
按照整个流程来划分，可以分为**建立连接**、**处理业务（读写）**、**关闭连接**三部分；下面以`EchoServer`作为例子进行比较。

#### 1. 建立连接

* Reactor
    `Reactor`模式中，我们在连接到来时，创建一个`Conn`对象，为它设置好**每种事件来临时对应的回调函数**，然后让他上树去监听它的事件：
    ```cpp
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

        // 设置好各个事件来临时的回调
        conn->setConnectionCallback(connectionCallback_);
        conn->setMessageCallback(messageCallback_);
        conn->setWriteCompleteCallback(writeCompleteCallback_);
        conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
        // 上树进行监听
        ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn)); 
    }
    ```

* Coroutine
`Co`模式中，连接到来时我们还是会创建一个`Conn`对象，不过对象生成时，我们会给它指定一个完整的任务：
```cpp
void CoTcpServer::startAccept() {
    while(true) {
        InetAddress peerAddr;
        int connfd = listenSock_->accept(peerAddr);
        if(connfd > 0) {
            TRACE("Accept of {}", peerAddr.getIpPortStr());
            std::string connName = hostName_ + "-" + hostIpPort_ + "#" + std::to_string(nextConnId_++);

            // 按照轮叫的方式选择一个Processor
            // 将connHandler_作为一个协程（任务）添加到Processor的队列中
            sched_->addTask(std::bind(connHandler_, 
                std::make_shared<CoTcpConnection>(connfd, 
                        listenAddr_, peerAddr, connName)));
        } 
    }
}
```

* 对比
  * 任务在协程中是作为一个整体添加的，而不是作为分散的回调函数来设置
  * 在```Reactor```中，**我们设置好了各种回调，等待事件到来内核通知，程序去做对应的工作**————所以在一开始我们就需要让fd上树监听，由此进入我们的“状态机”的跳转；而在```Co```中，我们不会手动地去让连接对应的fd上树，而是在该任务执行到被阻塞的情况时，自动地上树监听，并Yield。

#### 2. 处理业务（echo)
* ```Reactor``` 
正如上面所讲，Reactor中`Echo`业务的实现在回调函数中体现：

    * 首先是内核通知fd上有可读事件到来，于是channel回调Conn中的```handleRead()```

    ```cpp
    // 该函数处理内核层面的可读事件，将数据从内核缓冲区复制到用户缓冲区
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
    ```
    * 接下来，```onMessage()```被回调，我们将InputBuffer中的数据取出，计算长度，并且原封不动地发送回去

    ```cpp
    // onMsg中是我们的业务逻辑处理
    void onMessage(const TcpConnectionPtr& conn, Buffer& buf, Timestamp receiveTime) {    
        std::string msg(buf.retrieveAllAsString());
        std::cout << "New message from " << conn->getName() << ":" << msg << std::endl;
        conn->send(msg);
    }   
    ```
* `Co`
Co模式下，该协程只包含一个整体任务：
```cpp
void connHandler(CoTcpConnection::ptr conn) {
    RingBuffer::ptr buffer = std::make_shared<RingBuffer>();
    while(conn->recv(buffer) > 0) {
        conn->send(buffer);
    }
}
```
当该协程被执行时，它创建了一个Buffer对象；接下来**它尝试从该连接对应的fd中读取数据，然而可能此时并没有数据到来**————于是在Hook函数中，**它把该fd上树监听，并且主动将自己换出，让Processor执行下一个协程**：
```cpp
template<typename OriginFun, typename... Args>
static ssize_t ioHook(int fd, OriginFun origin_func, int event, Args&&... args) {
	ssize_t n;

	burger::net::Processor* proc = burger::net::Processor::GetProcesserOfThisThread();
	if (!burger::net::isHookEnable()) {
		return origin_func(fd, std::forward<Args>(args)...);
	}

retry:
	do {
		n = origin_func(fd, std::forward<Args>(args)...);
	} while (n == -1 && errno == EINTR);

	if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {

		//注册事件，事件到来后，将当前上下文作为一个新的协程进行调度
		proc->updateEvent(fd, event, burger::Coroutine::GetCurCo());
		burger::Coroutine::GetCurCo()->setState(burger::Coroutine::State::HOLD);
		burger::Coroutine::Yield();

		if(proc->stoped()) return 8;  // 当processor stop后，直接返回并且没有while，优雅走完函数并析构
		
		goto retry;
	}

	return n;
}
```
**当事件到来后，CoEpoll又回重新将该协程添加到队列中，当下次执行该协程时，则可以成功读取到数据**，并且继续我们的业务流程。

* 对比：
    * 虽然二者都存在使用Epoll监听事件到来的设计，**但是在协程中由于hook和上下文的存在，我们的业务逻辑完全是同步的写法**；
    * Reactor中，是**先上树监听，才有有事件到来，才有对应的执行动作（回调函数）**；Co中，是**先有执行动作（任务），才可能会有阻塞情况，才会上树监听**。

## 模型对比 todo

https://zhuanlan.zhihu.com/p/362621806