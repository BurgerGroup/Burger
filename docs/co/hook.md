## Hook模块

我们的Hook粒度是线程级别的

所谓系统函数hook，简单来说，就是替换原有的系统函数，例如read、write等，替换为自己的逻辑。

在内核级线程中，如果线程被io等操作阻塞，内核会自动帮我们切换线程；用户级线程（也即是协程）在被阻塞时，内核没有感知，就需要我们自己来进行切换————通过返回值+错误码即可判断是否是阻塞状态。

简言之，hook使我们的processor有了看上去类似系统自动调度线程的**自动调度协程的能力**（虽然本质上是协程自己决定换出）

## Hook 具体实现

操作系统提供了运行时加载和链接共享库的方式。其可以通过共享库的方式来让引用程序在下次运行时执行不同的代码，这也为应用程序 Hook 系统函数提供了基础。

Unix-like 系统提供了 dlopen，dlsym 系列函数来供程序在运行时操作外部的动态链接库，从而获取动态链接库中的函数或者功能调用（运行时库打桩）。我们通过 dlsym 来包装系统函数，从而实现 Hook 的功能。

比较核心的部分就是改变io的逻辑，当通过错误码知道在阻塞时，便注册事件，然后Yield出去。
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

		//注册事件，事件到来后，poll出来将当前上下文再次入队执行
		proc->updateEvent(fd, event);
		burger::Coroutine::GetCurCo()->setState(burger::Coroutine::State::HOLD);
		burger::Coroutine::Yield();

		if(proc->stoped()) return 8;  // 当processor stop后，直接返回并且没有while，优雅走完函数并析构
		
		goto retry;
	}

	return n;
}
```

## 关于 dlsym RTLD_NEXT

man page 描述:

There are two special pseudo-handles, RTLD_DEFAULT and RTLD_NEXT. The former will find the first occurrence of the desired symbol using the default library search order. The latter will find the next occurrence of a function in the search order after the current library. This allows one to provide a wrapper around a function in another shared library.

```cpp
// 我们摘录 libgo 源码部分来看

typedef unsigned int (*sleep_t)(unsigned int seconds);

sleep_t sleep_f = NULL;

/* find the next occurrence of a function in the search order
   找到的就是加载进来libc.so的函数地址，然后将其函数指针给xx_f
*/
sleep_f = (sleep_t)dlsym(RTLD_NEXT, "sleep");  

unsigned int sleep(unsigned int seconds) {
    if (!sleep_f) initHook();

    Task* tk = Processer::GetCurrentTask();
    DebugPrint(dbg_hook, "task(%s) hook sleep(seconds=%u). %s coroutine.",
            tk->DebugInfo(), seconds,
            Processer::IsCoroutine() ? "In" : "Not in");

    if (!tk)
        return sleep_f(seconds);

    Processer::Suspend(std::chrono::seconds(seconds));
    Processer::StaticCoYield();
    return 0;
}
```
通过这样的机制完成了对库函数的包装:
* 在本模块中，肯定首先找到的是我们自定义的函数，而通过`dlsym`保存库函数地址
* 包装逻辑是去判断本线程是否hook住，没有hook的话我们要去找到库中的函数，就是我们这里存储的函数地址xx_f
* 由于我们可以接触源代码，hook本质上是编译时库打桩和运行时库打桩的结合，所以不需要分开编译、也不需要额外生成动态库就能达到覆盖系统函数的作用。
