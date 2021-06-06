## Hook模块

我们的Hook粒度是线程级别的

所谓系统函数hook，简单来说，就是替换原有的系统函数，例如read、write等，替换为自己的逻辑。

在内核级线程中，如果线程被io等操作阻塞，内核会自动帮我们切换线程；用户级线程（也即是协程）在被阻塞时，内核没有感知，就需要我们自己来进行切换————通过返回值+错误码即可判断是否是阻塞状态。

简言之，hook使我们的processor有了看上去类似系统自动调度线程的**自动调度协程的能力**（虽然本质上是协程自己决定换出）

## Hook 具体实现

操作系统提供了运行时加载和链接共享库的方式。其可以通过共享库的方式来让引用程序在下次运行时执行不同的代码，这也为应用程序 Hook 系统函数提供了基础。

Unix-like 系统提供了 dlopen，dlsym 系列函数来供程序在运行时操作外部的动态链接库，从而获取动态链接库中的函数或者功能调用。我们通过 dlsym 来包装系统函数，从而实现 Hook 的功能。

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