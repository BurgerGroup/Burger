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

```
// 一共 72 + bytes
/****************************************************************************************
 *                                                                                      *
 *  ----------------------------------------------------------------------------------  *
 *  |    0    |    1    |    2    |    3    |    4     |    5    |    6    |    7    |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x0   |   0x4   |   0x8   |   0xc   |   0x10   |   0x14  |   0x18  |   0x1c  |  *
 *  ----------------------------------------------------------------------------------  *
 *  | fc_mxcsr|fc_x87_cw|        R12        |         R13        |        R14        |  *
 *  ----------------------------------------------------------------------------------  *
 *  ----------------------------------------------------------------------------------  *
 *  |    8    |    9    |   10    |   11    |    12    |    13   |    14   |    15   |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x20  |   0x24  |   0x28  |  0x2c   |   0x30   |   0x34  |   0x38  |   0x3c  |  *
 *  ----------------------------------------------------------------------------------  *
 *  |        R15        |        RBX        |         RBP        |        RIP        |  *
 *  ----------------------------------------------------------------------------------  *
 *  ----------------------------------------------------------------------------------  *
 *  |    16   |   17    |                                                            |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x40  |   0x44  |                                                            |  *
 *  ----------------------------------------------------------------------------------  *
 *  |        EXIT       |                                                            |  *
 *  ----------------------------------------------------------------------------------  *
 *                                                                                      *
 ****************************************************************************************/
 *
 *             -------------------------------------------------------------------------------
 * context:   |   r12   |   r13   |   r14   |   r15   |   rbx   |   rbp   |   rip   |   end   | ...
 *             -------------------------------------------------------------------------------
 *            8         16        24        32        40        48        56        64        |
 *            0x8       0x10     0x18      0x20      0x28      0x30      0x38      0x40       |  16-align for macosx
 *                                                                                  |
 *                                                                       rsp when jump to function


### make_fcontext

/*
 * @param stackdata     the stack data (rdi)
 * @param stacksize     the stack size (rsi)
 * @param func          the entry function (rdx)
 *
 * @return              the context pointer (rax)
*/
// notes: 当参数少于7个时， 参数从左到右放入寄存器: rdi, rsi, rdx, rcx, r8, r9。
// rax用于第一个返回寄存器
fcontext_t make_fcontext( void * sp, std::size_t size, void (* fn)( intptr_t) );

// 代码实现
make_fcontext:
    // 保存栈顶指针到rax
    movq  %rdi, %rax

    /* 先对栈指针进行16字节对齐
     *
     *
     *             ------------------------------
     * context:   | retaddr |    padding ...     |
     *             ------------------------------
     *            |         |
     *            |     此处16字节对齐
     *            |
     *  esp到此处时，会进行ret
     *
     * 这么做，主要是因为macosx下，对调用栈布局进行了优化，在保存调用函数返回地址的堆栈处，需要进行16字节对齐，方便利用SIMD进行优化
     */
     // andq $-16, %rax 表示低4位取0。 -16 的补码表示为0xfffffffff0.
    andq  $-16, %rax

    // 保留context需要的一些空间，因为context和stack是在一起的，stack底指针就是context
    leaq  -0x48(%rax), %rax

    // 保存func函数地址到context.rip
    movq  %rdx, 0x38(%rax)

    // 保存fpu 总大小8字节。
    // 操作系统使用这些专用寄存器在任务切换时保存状态信息。
    stmxcsr  (%rax)
    fnstcw   0x4(%rax)

    // 计算finish的绝对地址，保存到栈的0x40位置。  
    // leaq finish(%rip), %rcx 表示finish是相对位置+rip 就是finish的函数的地址。 
    leaq  finish(%rip), %rcx
    movq  %rcx, 0x40(%rax)

    // 返回,rax 作为返回值，目前的指向可以当做新栈的栈顶，相当于rsp
    ret /* return pointer to context-data */

finish:
    /* exit code is zero */
    xorq  %rdi, %rdi
    /* exit application */
    call  _exit@PLT
    hlt


// 为什么会预留72字节大小。首先知道jump_fcontext 在新栈需要 pop 的大小为，fpu(8字节）+ rbp rbx r12 ~ r15 (8*6 = 48 字节) = 56 字节。还会继续POP rip 8 字节，所以可以看到第二步中 movq %rdx, 0x38(%rax)，就是将rip 保存到这个位置。
目前已经64字节了，栈还有存储什么呢，协程（fn 函数）运行完成后会退出调用ret,其实就是POP到 rip.所以保存是finish 函数指针 大小8字节。总共 72 字节。
```

### jump_fcontext

```asm
jump_fcontext:  
    // 保存寄存器
    pushq  %rbp  /* save RBP */
    pushq  %rbx  /* save RBX */
    pushq  %r15  /* save R15 */
    pushq  %r14  /* save R14 */
    pushq  %r13  /* save R13 */
    pushq  %r12  /* save R12 */

    // 预留fpu 8个字节空间
    leaq  -0x8(%rsp), %rsp

    // 判断是否保存fpu
    // rcx是第四个参数，判断是否等于0。如果为0，跳转到1标示的位置。也就是preserve_fpu 。
    // 当preserve_fpu = true 的时候，需要执行2个指令是将浮点型运算的2个32位寄存器数据保存到第2步中预留的8字节空间。
    cmp  $0, %rcx
    je  1f

    stmxcsr  (%rsp)
    fnstcw   0x4(%rsp)

1:
    // 修改rsp 此时已经改变到其他栈
    // 将rsp 保存到第一参数（第一个参数保存在rdi）指向的内存。
    // fcontext_t *ofc 第一参数ofc指向的内存中保存是 rsp 的指针
    movq  %rsp, (%rdi)

    // 实现了将第二个参数复制到 rsp.
    movq  %rsi, %rsp

    // 判断是否保存了fpu，如果保存了就恢复保存在nfx 栈上的 fpu相关数据到响应的寄存器。
    cmp  $0, %rcx
    je  2f

    ldmxcsr  (%rsp)
    fldcw  0x4(%rsp)

2:
    //将rsp 存储的地址+8（8字节fpu），按顺序将栈中数据恢复到寄存器中。
    leaq  0x8(%rsp), %rsp

    popq  %r12  /* restrore R12 */
    popq  %r13  /* restrore R13 */
    popq  %r14  /* restrore R14 */
    popq  %r15  /* restrore R15 */
    popq  %rbx  /* restrore RBX */
    popq  %rbp  /* restrore RBP */

    // 设置返回值，实现指令跳转。
    popq  %r8

    /* use third arg as return-value after jump */
    movq  %rdx, %rax
    /* use third arg as first arg in context function */
    movq  %rdx, %rdi

    /* indirect jump to context */
    jmp  *%r8
.size jump_fcontext,.-jump_fcontext

// todo
// https://segmentfault.com/a/1190000019154852 

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


## reference

https://www.bbsmax.com/A/nAJvOaG85r/

https://stackoverflow.com/questions/52525744/understanding-gccs-alloca-alignment-and-seemingly-missed-optimization

https://zhuanlan.zhihu.com/p/363971930

https://segmentfault.com/a/1190000019154852