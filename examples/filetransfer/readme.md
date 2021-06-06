## 用发送文件说明CoTcpConnection::send()的使用

send 返回值为void,用户不必关心调用send()时发送多少字节，会保证把数据发送给对方

send 非阻塞，即便TCP发送窗口满了，也绝不会阻塞当前调用的线程

send 线程安全  -- 注意原子性

## 探究 原子性

reactor模式的send具有原子性

多个线程同时调用，消息之间不会混叠或交织

假设两个线程同各自发送上一条任意长度的消息，要么先a后b，or 先b后a，不会出现a的前一半，b，a的后一半

在rector模式中，原子性是由outbuffer来保证的，发送a若未发送完全，那么加入outbuffer中，若b也未发送完，那么也加入outbuffer,而在outbuffer中数据的先后是有序的

而协程模式虽然我们的send无原子性，如果对应多个task，我第一个task send导致了发送收缓冲区满，我切换出去epoll关注，执行第二个task，这样导致了我们消息的交织，但是这种场景本来就不符合我们同步写法，我们一个conn对应一个task，就是我们的connHandler，我们的逻辑中有一个send，这个send执行完毕，我们才进行下一轮的数独求解。

## 三种文件传输模式

1. 一次性读入内存

- 在reactor模式下，当filecontent比较大，不会一次性将数据发送到内核缓冲区，这时将剩余的数据拷贝到应用层的outputbuffer中，当内核缓冲区中的数据发送出去后，可写事件产生，从outputbuffer中取出数据继续发送

而此处send后立马shutdown，这里会出问题吗？ 

不会，因为当前不在写的状态，才会去关闭写端

```cpp
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

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if(!channel_->isWriting()) {  
        socket_->shutdownWrite();  
    }
}
```

虽然我们这里没去shutdownWrite，但是我们化悲愤state设置为了Status::kDisconnecting

一旦将所有数据发送完毕，会产生handleWrite事件

```cpp
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
```

此处如果缓冲区已经清空，状态又是Status::kDisconnecting，那么在此调用shutdownInLoop关闭写端

此处显示出这个状态的作用

- 而协程模式下，我们会Yield出去，然后事件上epoll关注，等到可写事件触发，切换回上下文

而此处的shutdown是得在send的上下文完毕后才执行，所以不存在reactor的考虑和状态设计

2. 一块一块发送, 并用shared_ptr管理FILE*

我们使用setContext和getContext来保存conn的用户上下文(此处是std::shared_ptr<FILE>)

reactor 通过 conn->setContext(fp), 将conn和fp绑定，通过这种方法，我们就不需要一个map容器来管理对应关系    

3. 同2，


## 测试客户端程序  -- todo : 需要大文件测试
