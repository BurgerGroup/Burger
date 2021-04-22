#include "Processor.h"
#include "SocketsOps.h"
#include "burger/base/Util.h"
#include "burger/base/Log.h"
#include "Hook.h"

using namespace burger;
using namespace burger::net;

namespace {
const int kEpollTimeMs = 10000;   // epoll超时10s

static thread_local Processor* t_proc = nullptr;
}


Processor::Processor(Scheduler* scheduler) 
    : scheduler_(scheduler),
    epoll_(this),
    wakeupFd_(sockets::createEventfd()) {
	// 当有新事件来时唤醒Epoll 协程
    addTask([&]() {
        while (!stop_) {
            if (consumeWakeUp() < 0) {
                ERROR("read eventfd : {}", util::strerror_tl(errno));
                break;
            }
        }
    }, "Wake");
    Coroutine::GetCurCo();  // todo need this?
}

// https://zhuanlan.zhihu.com/p/321947743
Processor::~Processor() = default;

void Processor::run() {
	if (GetProcesserOfThisThread() != nullptr) {
    	CRITICAL("Run two processers in one thread");
	} else {
		t_proc = this;
	}
	setHookEnabled(true);
	Coroutine::ptr cur;

	//没有可以执行协程时调用epoll协程
	Coroutine::ptr epollCo = std::make_shared<Coroutine>(std::bind(&CoEpoll::poll, &epoll_, kEpollTimeMs), "Epoll");

	while (!stop_) {
		{
            std::lock_guard<std::mutex> lock(mutex_);
			//没有协程时执行epoll协程
			if (coList_.empty()) {
				cur = epollCo;
				epoll_.setEpolling(true);
			} else {
                //  todo : 此处可优化
				for (auto it = coList_.begin();
				        it != coList_.end(); ++it) {
					cur = *it;
					coList_.erase(it);
					break;
				}
			}
		}
		cur->swapIn();
		if (cur->getState() == Coroutine::State::TERM) {
			load_--;
		}
	}
}

void Processor::stop() {
	stop_ = true; 
	if(epoll_.isEpolling()) {
		wakeupEpollCo();
	}
}

void Processor::addTask(Coroutine::ptr co) {
    std::lock_guard<std::mutex> lock(mutex_);
    coList_.push_back(co);
    load_++;
    if(epoll_.isEpolling()) {
        wakeupEpollCo();
    }
}

void Processor::addTask(const Coroutine::Callback& cb, std::string name) {
    addTask(std::make_shared<Coroutine>(std::move(cb), name));
}

void Processor::updateEvent(int fd, int events, Coroutine::ptr co) {
	epoll_.updateEvent(fd, events, co);
}

void Processor::removeEvent(int fd) {
	epoll_.removeEvent(fd);
}

Processor* Processor::GetProcesserOfThisThread() {
    return t_proc;
}

void Processor::wakeupEpollCo() {
	uint64_t one = 1;
	ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
	if(n != sizeof one) {
		ERROR("writes {} bytes instead of 8", n);
	}
}

ssize_t Processor::consumeWakeUp() {
	uint64_t one;
	ssize_t n = ::read(wakeupFd_, &one, sizeof one);
	if(n != sizeof one) {
		ERROR("READ {} instead of 8", n);
	}
	return n;
}


