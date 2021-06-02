## 协程杂记

## 一些概念

### 对称和非对称协程

对称与非对称最主要的区别在于是否存在传递程序控制权的行为

Coroutine可以分为两种

1. 非对称式(asymmetric)协程

之所以被称为非对称的，是因为它提供了两种传递程序控制权的操作：

coroutine.resume - (重)调用协程

coroutine.yield - 挂起协程并将程序控制权返回给协程的调用者

一个非对称协程可以看做是从属于它的调用者的，二者的关系非常类似于例程(routine)与其调用者之间的关系。

我们Burger框架实现的是非对称协程

```
Coroutine::GetCurCo()

Thread -> main_co <-----------> sub_co
            |
            |
          sub_co

main_co 负责切换，回收, 不分配栈空间
```

2. 对称式(symmetric)协程

对称式协程的特点是只有一种传递程序控制权的操作coroutine.transfer即将控制权直接传递给指定的协程。

### 对比

1. 对称式协程机制可以直接指定控制权传递的目标，拥有极大的自由，但得到这种自由的代价却是牺牲程序结构。

如果程序稍微复杂一点，那么即使是非常有经验的程序员也很难对程序流程有全面而清晰的把握。

这非常类似goto语句，它能让程序跳转到任何想去的地方，但人们却很难理解充斥着goto的程序

2. 非对称式协程具有良好的层次化结构关系，(重)启动这些协程与调用一个函数非常类似：被(重)启动的协程得到控制权开始执行，然后挂起(或结束)并将控制权返回给协程调用者。这与结构化编程风格是完全一致的

非对称的api实现更接近于传统的函数调用/返回流程（协程A不能自顾自的就切换堆栈切换去B了而是要“返回”去某个实体再切换去B）

go语言的协程就是对称线程，而腾讯的libco提供的协议就是非对称协程

### 无栈协程（stackless）和有栈协程（stackful）

有栈（stackful）协程，例如 goroutine；

无栈（stackless）协程，例如 async/await。

我们此处讨论x86 32位系统， DrawSquare 是 caller，DrawLine 是 callee

![stack](https://upload.wikimedia.org/wikipedia/commons/thumb/d/d3/Call_stack_layout.svg/342px-Call_stack_layout.svg.png)

Stack Pointer 即栈顶指针，总是指向调用栈的顶部地址，该地址由 esp 寄存器存储；Frame Pointer 即基址指针，总是指向当前栈帧（当前正在运行的子函数）的底部地址，该地址由 ebp 寄存器存储

Return Address 则在是 callee 返回后，caller 将继续执行的指令所在的地址；而指令地址是由 eip 寄存器负责读取的，且 eip 寄存器总是预先读取了当前栈帧中下一条将要执行的指令的地址。

[stack](https://www.bilibili.com/video/BV1By4y1x7Yh?from=search&seid=5540660959159115596)


## 探究问题 ：协程框架的效率问题

相对于我们通常使用的异步回调模式网络编程，协程框架并不能对于网络编程带来效率的提升，反而会稍微损失一些效率，因为协程的创建和切换有个开销

网络框架真正的性能中流砥柱是EPOLL，而协程可以看做是一个特殊的函数，当遇到阻塞就yield出去，准备好再resume回来到这个节点继续执行。

## 协程网络框架的优势 - 异步回调模型面临的问题

异步回调模型中，一个业务流程中每一个阻塞I/O的节点，都需要进行切断业务处理流程、保存当前处理的上下文（用户上下文）、设置回调函数，然后等待I/O完成后，再恢复上下文、继续处理业务流程。这样的流程带来了一些问题

1. 每个流程都要定义一个上下文实体，以便手动保存与恢复。

举个例子 

```cpp
// https://github.com/chenshuo/muduo/blob/master/examples/filetransfer/download2.cc
void onConnection(const TcpConnectionPtr& conn)
{
  LOG_INFO << "FileServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
  if (conn->connected())
  {
    ...
    FILE* fp = ::fopen(g_file, "rb");
    if (fp)
    {
      conn->setContext(fp);
      ...
    }
  }
  else
  {
    if (!conn->getContext().empty())
    {
      FILE* fp = boost::any_cast<FILE*>(conn->getContext());
      ...
    }
  }
}

void onWriteComplete(const TcpConnectionPtr& conn)
{
  FILE* fp = boost::any_cast<FILE*>(conn->getContext());
  ...
  else
  {
    ::fclose(fp);
    fp = NULL;
    conn->setContext(fp);
    conn->shutdown();
    LOG_INFO << "FileServer - done";
  }
}

```

在muduo中，TcpConnection连接时setContext保存上下文，在其他状态比如断开连接,写完或onMessage时可以getContext拿出来使用。

而我们Burger中不需要切断流程，所以不需要保护上下文，所有的上下文都在我们每个connection的ConnHandler中

```cpp 
// https://github.com/BurgerGroup/Burger/blob/main/examples/filetransfer/download2.cc
void connHandler(const CoTcpConnectionPtr& conn) {
    INFO("FileServer - Sending file {} to {}", g_file, conn->getPeerAddr().getIpPortStr());
    FILE* fp = ::fopen(g_file, "rb");
    if (fp) {
        char buf[kBufSize];
        size_t nread = 0;
        do {
            nread = ::fread(buf, 1, sizeof buf, fp);
            conn->send(buf, nread);
        } while(nread > 0);
        ::fclose(fp);
        fp = nullptr;
        conn->shutdown();
        INFO("FileServer - done");
    } else {
        conn->shutdown();
        INFO("FileServer - no such file");
    }
}

```

2. 由于每次回调都会切断栈上变量的生命周期，故而导致需要延续使用的变量必须申请到堆上，并手动存入上下文实体中。

3. 在C/C++这种无GC的语言中，碎片化的流程给内存管理也带来了更多挑战。

4. 由于回调式的逻辑是“不知何时会被触发”，用户状态管理也会有更多挑战。

所以像muduo的TcpConnection特别复杂，其复杂性来源于他有太多的状态。

比如他的handleWrite中，要考虑channel的状态，还要考虑outputBuffer是否发送完，还要考虑现在自身的状态，是否该去优雅退出


```cpp 
// https://github.com/chenshuo/muduo/blob/master/muduo/net/TcpConnection.cc
void TcpConnection::handleWrite()
{
  loop_->assertInLoopThread();
  if (channel_->isWriting())
  {
    ssize_t n = sockets::write(channel_->fd(),
                               outputBuffer_.peek(),
                               outputBuffer_.readableBytes());
    if (n > 0)
    {
      outputBuffer_.retrieve(n);
      if (outputBuffer_.readableBytes() == 0)
      {
        channel_->disableWriting();
        if (writeCompleteCallback_)
        {
          loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
        }
        if (state_ == kDisconnecting)
        {
          shutdownInLoop();
        }
      }
    }
    else
    {
      LOG_SYSERR << "TcpConnection::handleWrite";
      // if (state_ == kDisconnecting)
      // {
      //   shutdownInLoop();
      // }
    }
  }
  else
  {
    LOG_TRACE << "Connection fd = " << channel_->fd()
              << " is down, no more writing";
  }
}
```

而我们Burger中就不需要考虑这些问题，我们就是同步的写法

```cpp 
// https://github.com/BurgerGroup/Burger/blob/main/burger/net/CoTcpConnection.cc
void CoTcpConnection::sendInProc(const char* start, size_t sendSize) {
    if(quit_) {
        WARN("Disconnected, give up writing");
        return;
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
                ERROR("CoTcpConnection can't send errno = {}", errno);
                if(errno == EPIPE || errno == ECONNRESET) {
                    WARN("peer is disconnected..");
                }
                // 本端关闭，对端关闭都要注意quit_的改变
                quit_ = true;
                break;
            }
        }
    } 
}
```

我们此处如果没发完，就yield出去等下次到resume到此处继续执行即可，而不需要像muduo那样先把数据append到outputBuffer中，将channel设置关注write，当等发送缓冲区空了后触发，然后调用handleWrite将outputBuffer中数据继续发送出去。

## reference 

https://github.com/hunterhang/LibcoLearning

https://github.com/wangbojing/NtyCo

https://www.codenong.com/cs106804383/

https://zhuanlan.zhihu.com/p/362621806

https://mthli.xyz/stackful-stackless/