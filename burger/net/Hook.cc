#include "Processor.h"
#include "Hook.h"
#include "Scheduler.h"
#include "CoEpoll.h"
#include "SocketsOps.h"
#include "burger/base/Timestamp.h"
#include "CoTimerQueue.h"
#include <dlfcn.h>
#include <errno.h>
#include <memory>
#include <string.h>

namespace burger {
namespace net {

#define DLSYM(name) \
		name ## _f = (name ## _t)::dlsym(RTLD_NEXT, #name);


// valgrind check 32 bytes leak from dlsym : 
// https://bugzilla.redhat.com/show_bug.cgi?id=1624387

struct HookIniter {
	HookIniter() {
		DLSYM(sleep);
		DLSYM(accept);
		DLSYM(accept4);
		DLSYM(connect);
		DLSYM(read);
		DLSYM(readv);
		DLSYM(recv);
		DLSYM(recvfrom);
		DLSYM(recvmsg);
		DLSYM(write);
		DLSYM(writev);
		DLSYM(send);
		DLSYM(sendto);
		DLSYM(sendmsg);
	}
};

static HookIniter hook_initer;

static thread_local bool t_hook_enabled = false;

bool isHookEnable() {
	return t_hook_enabled;
}

void setHookEnabled(bool flag) {
	t_hook_enabled = flag;
}

} // namespace net
} // namespace burger

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

extern "C" {
#define HOOK_INIT(name) \
		name ## _t name ## _f = nullptr;

HOOK_INIT(sleep)
HOOK_INIT(accept)
HOOK_INIT(accept4)
HOOK_INIT(connect)
HOOK_INIT(read)
HOOK_INIT(readv)
HOOK_INIT(recv)
HOOK_INIT(recvfrom)
HOOK_INIT(recvmsg)
HOOK_INIT(write)
HOOK_INIT(writev)
HOOK_INIT(send)
HOOK_INIT(sendto)
HOOK_INIT(sendmsg)



unsigned int sleep(unsigned int seconds) {
	burger::net::Processor* proc = burger::net::Processor::GetProcesserOfThisThread();
	if (!burger::net::isHookEnable()) {
		return sleep_f(seconds);
	}
	// fixme how about mainCo
	proc->getTimerQueue()->addTimer(burger::Coroutine::GetCurCo(), burger::Timestamp::now() + static_cast<uint64_t>(seconds * burger::Timestamp::kMicroSecondsPerSecond));
	burger::Coroutine::Yield();
	return 0;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	return ioHook(sockfd, accept_f, burger::net::CoEpoll::kReadEvent, addr, addrlen);
}

int accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags) {
	return ioHook(sockfd, accept4_f, burger::net::CoEpoll::kReadEvent, addr, addrlen, flags);
}

int connect(int sockfd, const struct sockaddr *addr,
				                   socklen_t addrlen) {
	burger::net::Processor* proc = burger::net::Processor::GetProcesserOfThisThread();
	if (!burger::net::isHookEnable()) {
		return connect_f(sockfd, addr, addrlen);
	}

	int ret = ::connect_f(sockfd, addr, addrlen);

	if (ret == -1 && errno == EINPROGRESS) {
		proc->updateEvent(sockfd, burger::net::CoEpoll::kWriteEvent);
		burger::Coroutine::GetCurCo()->setState(burger::Coroutine::State::HOLD);
		burger::Coroutine::Yield();

		int err = burger::net::sockets::getSocketError(sockfd);
		if (err == 0) {
			return 0;
		} else {
			return -1;
		}
	}
	
	return ret;
}


ssize_t read(int fd, void *buf, size_t count) {
	return ioHook(fd, read_f, burger::net::CoEpoll::kReadEvent, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
	return ioHook(fd, readv_f, burger::net::CoEpoll::kReadEvent, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
	return ioHook(sockfd, recv_f, burger::net::CoEpoll::kReadEvent, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, 
				struct sockaddr *src_addr, socklen_t *addrlen) {
	return ioHook(sockfd, recvfrom_f, burger::net::CoEpoll::kReadEvent, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
	return ioHook(sockfd, recvmsg_f, burger::net::CoEpoll::kReadEvent, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
	return ioHook(fd, write_f, burger::net::CoEpoll::kWriteEvent, buf, count);
}


ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
	return ioHook(fd, writev_f, burger::net::CoEpoll::kWriteEvent, iov, iovcnt);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
	return ioHook(sockfd, send_f, burger::net::CoEpoll::kWriteEvent, buf, len, flags);
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, 
		const struct sockaddr *dest_addr, socklen_t addrlen) {
	return ioHook(sockfd, sendto_f, burger::net::CoEpoll::kWriteEvent, buf, len, flags, dest_addr, addrlen);
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags) {
	return ioHook(sockfd, sendmsg_f, burger::net::CoEpoll::kWriteEvent, msg, flags);	
}

}

