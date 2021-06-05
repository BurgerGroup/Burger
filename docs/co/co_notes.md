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

## 无栈协程（stackless）和有栈协程（stackful）

有栈（stackful）协程，例如 goroutine；

无栈（stackless）协程，例如 async/await。

有栈 和 无栈 的含义不是指协程在运行时是否需要栈

而是指协程是否可以在其任意嵌套函数中被挂起，此处的嵌套函数读者可以理解为子函数、匿名函数等。显然有栈协程是可以的，而无栈协程则不可以

此处c语言的stack可见资料:

[stack](https://www.bilibili.com/video/BV1By4y1x7Yh?from=search&seid=5540660959159115596)

For example :

```cpp
int callee() { 
    int x = 0; 
    return x;  
}

int caller() { 
    callee(); 
    return 0; 

}
```

通过[godbolt](https://godbolt.org/)编译参数-m32

```asm
callee():
        push    ebp
        mov     ebp, esp
        sub     esp, 16  // 关于此处为何为16，https://stackoverflow.com/questions/49391001/why-does-the-x86-64-amd64-system-v-abi-mandate-a-16-byte-stack-alignment
        mov     DWORD PTR [ebp-4], 0
        mov     eax, DWORD PTR [ebp-4]
        leave
        ret

    // "leave" 等价于如下两条指令：
    // 6. 将调用栈顶部与 callee 栈帧底部对齐，释放 callee 栈帧空间
    // 7. 将之前保存的 caller 的栈帧底部地址出栈并赋值给 ebp
    // movl %ebp, %esp
    // popl %ebp

    // "ret" 等价如下指令：
    // 8. 将之前保存的 caller 的 return address 出栈并赋值给 eip，
    //    即 caller 的 "movl $0, %eax" 这条指令所在的地址
    // popl eip
caller():
        push    ebp
        mov     ebp, esp
        call    callee()
        mov     eax, 0
        pop     ebp
        ret

    // "call callee" 等价于如下两条指令：
    // 1. 将 eip 存储的指令地址入栈保存；
    //    此时的指令地址即为 caller 的 return address，
    //    即 caller 的 "movl $0, %eax" 这条指令所在的地址
    // 2. 然后跳转到 callee
    pushl %eip
    jmp callee
```

可画图分析。

## 有栈协程

实现一个协程的关键点在于如何保存、恢复和切换上下文。

已知函数运行在调用栈上，保存上下文即是保存从这个函数及其嵌套函数的（连续的）栈帧存储的值，以及此时寄存器存储的值；

恢复上下文即是将这些值分别重新写入对应的栈帧和寄存器；

切换上下文便是保存当前正在运行的函数的上下文，恢复下一个将要运行的函数的上下文。

有栈协程是可以在其任意嵌套函数中被挂起的——毕竟它都能保存和恢复自己完整的上下文了，那自然是在哪里被挂起都可以

### 无栈协程

与有栈协程相反，无栈协程不会为各个协程开辟相应的调用栈。无栈协程通常是 基于状态机或闭包 来实现。

状态机记录上次协程挂起时的位置，并基于此决定协程恢复时开始执行的位置。这个状态必须存储在栈以外的地方，从而避免状态与栈一同销毁。

某种角度说，协程与函数无异，只不过前者会记录上次终端的位置，从而可以实现恢复执行的能力

在实际过程中，恢复后的执行流可能会用到中断前的状态，因此无栈协程会将保存完整的状态，这些状态会被存储到堆上。

可见一种无栈协程的实现[无栈协程实现](https://mthli.xyz/coroutines-in-c/)

### 有栈协程 vs 无栈协程 

由于不需要切换栈帧，无栈协程的性能倒是比有栈协程普遍要高一些, 协程恢复时，需要将运行时上下文从堆中拷贝至栈中，这里也存在一定的开销。

但是无栈协程的实现还是存在比较多的限制，最大缺点就是，它无法实现在任意函数调用层级的位置进行挂起。

代码例子:

```cpp
// libco有栈协程
void* test(void* para){
	co_enable_hook_sys();
	int i = 0;
	poll(0, 0, 0. 1000); // 协程切换执行权，1000ms后返回
	i++;
	poll(0, 0, 0. 1000); // 协程切换执行权，1000ms后返回
	i--;
	return 0;
}

int main(){
	stCoRoutine_t* routine;
	co_create(&routine, NULL, test, 0);// 创建一个协程
	co_resume(routine); 
	co_eventloop( co_get_epoll_ct(),0,0 );
	return 0;
}
```

```cpp
// 对应无栈协程
// 原本需要执行切换的语句处为界限，把函数划分为几个部分，并在某一个部分执行完以后进行状态转移，
// 在下一次调用此函数的时候就会执行下一部分，这样的话我们就完全没有必要像有栈协程那样显式的执行上下文切换了，
// 我们只需要一个简易的调度器来调度这些函数即可。
// 从执行时栈的角度来看，其实所有的协程共用的都是一个栈，即系统栈
// 因为是函数调用，我们当然也不必去显示的保存寄存器的值，
// 而且相比有栈协程把局部变量放在新开的空间上，无栈协程直接使用系统栈使得CPU cache局部性更好
// 同时也使得无栈协程的中断和函数返回几乎没有区别，这样也可以凸显出无栈协程的高效。
struct test_coroutine {
    int i;
    int __state = 0;
    void MoveNext() {
        switch(__state) {
        case 0:
            return frist();
        case 1:
            return second();
        case 2:
        	return third();
        }
    }
    void frist() {
        i = 0;
        __state = 1;
    }
    void second() {
        i++;
        _state = 2;
    }
    void third() {
    	i--;
    }
};
```

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

## 

## reference 

https://github.com/hunterhang/LibcoLearning

https://github.com/wangbojing/NtyCo

https://www.codenong.com/cs106804383/

https://zhuanlan.zhihu.com/p/362621806

https://mthli.xyz/stackful-stackless/

https://zhuanlan.zhihu.com/p/347445164