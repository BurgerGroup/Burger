## 协程模块

实现非对称协程

```
Coroutine::GetCurCo()

Thread -> main_co <-----------> sub_co
            |
            |
          sub_co

main_co 负责切换，回收, 不分配栈空间
```

协程调度模块scheduler

如何让协程在线程间切换

```
    1    --     N      1 -- M
scheduler --> thread --> co

        N  - --------  M

1. 执行器Processor,每一个IO线程都有一个独一无二的Processor
2. 协程调度器，将协程，分配到线程(Processor)上去执行

```


## FAQ 

* 1. `CoEpoller`的作用？为什么源码里是wait？
    答：协程因为阻塞swapOut之后，需要`CoEpoller`来监听状态；在事件来临时，将对应的协程重新加入就绪队列。
    （简洁地说：没有协程可执行，就epoll监听事件，来临时获取对应协程）
    libgo中，是在processor中swapout，然后在就绪队列中获取下一个协程，没有协程时当然就是wait等待添加协程。

* 2. co从coQue里pop后，swapout时会重新加入队列吗？（不会的话是怎么再次调度到他的）
    答：在swapout时，有两种情况：
        1. 协程执行完毕；2.协程任务阻塞。而在阻塞时，协程会将需要监听的fd上树，等待准备好时再次执行。

* 3. Hook的作用
    答：在内核级线程中，如果线程被io等操作阻塞，内核会自动帮我们切换线程；用户级线程（也即是协程）在被阻塞时，内核没有感知，就需要我们自己来进行切换————通过返回值+错误码即可判断是否是阻塞状态。
    简言之，hook使我们的processor有了看上去类似系统自动调度线程的**自动调度协程的能力**（虽然本质上是协程自己决定换出）


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
  * 在```Reactor```中，**我们设置好了各种回调，等待事件到来内核通知，程序去做对应的工作**————所以在一开始我们就需要让fd上树监听，由此进入我们的“状态机”的跳转；而在```Co```中，我们不会手动地去让连接对应的fd上树，而是在该任务执行到被阻塞的情况时，自动地上树监听，并SwapOut。

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
    void onMessage(const TcpConnectionPtr& conn, Buffer& buf, Timestamp receiveTime) {    
        std::string msg(buf.retrieveAllAsString());
        std::cout << "New message from " << conn->getName() << ":" << msg << std::endl;
        conn->send(msg);
    }   
    ```