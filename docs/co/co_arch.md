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

## 执行器Processor 职责

多线程情况下Scheduler有一个mainProc 和多个workProc, mainProc的职责相当于一个包工头，因为mainProc所在线程主要负责accept连接(和客户见面交接)

```cpp 
// https://github.com/BurgerGroup/Burger/blob/main/burger/net/CoTcpServer.cc
void CoTcpServer::start() {
    if(started_.getAndSet(1) == 0) {
        sched_->startAsync();
        listenSock_->listen();
        sched_->addMainTask(std::bind(&CoTcpServer::startAccept, this), "Accept");
    }
}
```

然后将连接好的connFd生成conn, 和connHandler交给workProc去处理(打工人干活)

```cpp 
// https://github.com/BurgerGroup/Burger/blob/main/burger/net/CoTcpServer.cc
void CoTcpServer::startAccept() {
    while(started_.get()) {
        InetAddress peerAddr;
        int connfd = listenSock_->accept(peerAddr);
        if(connfd > 0) {
            ... 
            // 将conn交给一个sub processor
            Processor* proc = sched_->pickOneWorkProcessor();
            CoTcpConnection::ptr conn = std::make_shared<CoTcpConnection>(proc, connfd,
                        listenAddr_, peerAddr, connName);
            proc->addTask(std::bind(connHandler_, conn), "connHandler");
        } else {
            ... 
        }
}

```

当然如果整个公司只有这么一个包工头，那么连接和工作的事都由mainProc来干。

## Processor 细节

Processor名为运行器，是和协程的运行相关，其最核心功能在于run和addTask。

### Processor::addTask

addTask线程安全

```cpp 
void Processor::addTask(const Coroutine::Callback& cb, const std::string& name) {
    if(isInProcThread()) 
        addTask(resetAndGetCo(cb, name));
    else 
        addPendingTask(cb, name);
}
```

其中我们本线程内会调用resetAndGetCo()来包装callback成协程，我们这里有两个queue，一个是等待执行队列 runnableCoQue_，一个是执行完后放入的idleCoQue_ ，等待我们复用这些已经执行的co，避免重复malloc, free。 

跨线程调用addPendingTask(), 在run中由该线程来将其包装成co。

### Processor::run

```cpp
void Processor::run() {
    assertInProcThread();
    TRACE("Processor {} start running", fmt::ptr(this));
    stop_ = false;
	setHookEnabled(true);
	//没有可以执行协程时调用epoll协程
	Coroutine::ptr epollCo = std::make_shared<Coroutine>(std::bind(&CoEpoll::poll, &epoll_, kEpollTimeMs), "Epoll");
	epollCo->setState(Coroutine::State::EXEC);

	Coroutine::ptr cur;
	while (!stop_ || !runnableCoQue_.empty()) {
        //没有协程时执行epoll协程
        if (runnableCoQue_.empty()) {
            cur = epollCo;
            epoll_.setEpolling(true);
        } else {
            cur = runnableCoQue_.front();
            runnableCoQue_.pop();
        } 
		cur->swapIn();
		if (cur->getState() == Coroutine::State::TERM) {
            --load_;
            idleCoQue_.push(cur);
		}
        // 避免在其他线程添加任务时错误地创建多余的协程（确保协程只在processor中）
        addPendingTasksIntoQueue();
	}
    TRACE("Processor {} stop running", fmt::ptr(this));
	epollCo->swapIn();  // epoll进去把cb执行完
}
```

我们在run里创建一个epoll co，如果没其他协程等待执行就进入epoll::poll等待新事件到来，然后将跨线程和Scheduler添加的cb包装成co装入执行队列。

最后我们epollCo->swapIn()进入epoll 跳出循环执行完。


### Processor::other 

epoll的职责是关注事件，而我们此处都是加入队列等待执行co，并无直接和epoll交互，我们如何添加事件关注，详细见 [epoll](./co_epoll.md)

我们在创建时，processor会创建三个co，一个是每个线程所有的main_co, 一个是Wake co, 用来唤醒epoll的，还有一个是处理超时事件的timerQue。详见[coTimerQueue](./co_timer)

