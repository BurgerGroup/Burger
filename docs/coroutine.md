## 协程模块

### Burger协程介绍

Burger实现的是**非抢占式**的**非对称** **N : 1** 协程

在特定的位置（比如阻塞I/O的调用点。协程通常都以I/O作为调度基础），由当前协程自己主动出让CPU（SwapOut), 如何阻塞后切出是有Hook函数决定的(下面讲)

### 协程架构

```
Thread -> main_co <-----------> sub_co
            |
            |
          sub_co
```

因为我们是非对称协程，所以我们拥有两种协程

main_co, 只负责切换，回收, 不分配栈空间，每个线程拥有一个，通过无参构造函数构造

sub_co, 分配默认128k栈空间，包含一个callBack，是一个执行体Task

我们支持的是1:N模式，每一个线程对应一个协程队列，可以完全无锁编写同步风格代码，对于IO密集型友好


### 协程的创建和切换

现有的 C++ 协程库均基于两种方案：

利用汇编代码控制协程上下文的切换，以及利用操作系统提供的 API 来实现协程上下文切换

我们此处采用Boost.context 的两个API来作为协程的创建和切换

```C

intptr_t jump_fcontext( fcontext_t * ofc, fcontext_t nfc, intptr_t vp, bool preserve_fpu = false);

fcontext_t make_fcontext( void * sp, std::size_t size, void (* fn)( intptr_t) );

// 构造
ctx_ = make_fcontext(static_cast<char*>(stack_) + stackSize_, stackSize_, &Coroutine::RunInCo);

// 切换 
jump_fcontext(&curCo->ctx_, t_main_co->ctx_, 0);

```

### 协程的切换

我们的协程拥有三种状态，

```cpp 
enum class State {
    EXEC,  // 运行状态
    HOLD,  // 挂起状态
    TERM   // 未执行状态
};
```

我们通过swapIn 从main_co切换到当前协程co执行

通过 SwapOut 挂起当前正在执行的协程，切换到主协程执行

### 协程中callBack的执行

```cpp 
void Coroutine::RunInCo(intptr_t vp) {
    Coroutine::ptr cur = GetCurCo();
    DEBUG("Co : {} - {} running", cur->getCoId(), cur->getName());
    cur->cb_();
    cur->cb_ = nullptr;
    cur->setState(State::TERM);
    DEBUG("Co : {} - {} run end", cur->getCoId(), cur->getName());
    cur.reset();  // 防止无法析构
    Coroutine::SwapOut();   	//重新返回主协程
}
```

我们在创建协程的时候设置了这个函数为上下文环境，当swapIn到当前协程时候，跳转到此上下文执行，去调用callBack执行，执行结束后注意此处将cur.reset()，因为这里我们执行完SwapOut切换出去，这里协程就算完成结束了，而此处的上下文环境还保存着，这里局部智能指针还在其中有一个引用计数

## 协程调度模块Scheduler

```
    1    --     N      1 -- M
scheduler --> thread --> co
```

内部带有一个线程池, 每一个IO线程都有一个独一无二的执行器Processor

协程调度器将协程分配到线程(Processor)上去执行

我们对上层用户不暴露协程，上层只需要传入callBack，我们内部将callBack 交给一个工作Processor，并且调用的是proc->addPendingTask()，而不是直接addTask(下面Processor讲解为何)

```cpp
void Scheduler::addTask(const Coroutine::Callback& task, const std::string& name) {
    Processor* proc = pickOneWorkProcessor();
    assert(proc != nullptr);
    proc->addPendingTask(task, name);
}
```

总的来说，Scheduler与上层交互去增加任务，在内部将其分发给各个Processor处理，隐藏底层怎么处理的细节。

## 执行器Processor

多线程情况下Scheduler有一个mainProc 和多个workProc, mainProc的职责相当于一个包工头，因为mainProc所在线程主要负责accept连接，


## Hook模块

我们的Hook粒度是线程级别的，

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
		burger::Coroutine::SwapOut();

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