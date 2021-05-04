#include "Processor.h"
#include "SocketsOps.h"
#include "burger/base/Util.h"
#include "burger/base/Log.h"
#include "Hook.h"
#include "Scheduler.h"
#include "CoTimerQueue.h"

using namespace burger;
using namespace burger::net;

namespace {
const int kEpollTimeMs = 10000;   // epoll超时10s

static thread_local Processor* t_proc = nullptr;
}


Processor::Processor(Scheduler* scheduler) 
    : mainCo_(Coroutine::GetCurCo()),
    scheduler_(scheduler),
    epoll_(this),
    timerQueue_(util::make_unique<CoTimerQueue>(this)),
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

    addTask([&]() {
        while (!stop_) {
            timerQueue_->dealWithExpiredTimer();
        }
    }, "timerQue");
	DEBUG("Processor ctor");
}

// https://zhuanlan.zhihu.com/p/321947743
Processor::~Processor() {
	::close(wakeupFd_);
	DEBUG("Processor dtor");
};

void Processor::run() {
	if (GetProcesserOfThisThread() != nullptr) {
    	CRITICAL("Run two processers in one thread");
	} else {
		t_proc = this;
	}
    DEBUG("t_proc : {}", fmt::ptr(this));
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
	epollCo->swapIn();  // epoll进去把cb执行完
}

void Processor::stop() {
	stop_ = true; 
	if(epoll_.isEpolling()) {
		wakeupEpollCo();
	}
}

Coroutine::ptr Processor::getResetIdleCo(const Coroutine::Callback& cb, const std::string& name) {
        if(idleCoQue_.empty()) {
            return std::make_shared<Coroutine>(cb, name);
        } else {
            Coroutine::ptr co = idleCoQue_.front();
            idleCoQue_.pop();

            co->reset(cb);
            co->setName(name);
            return co;
        }
}

void Processor::addTask(Coroutine::ptr co, const std::string& name) {
    co->setState(Coroutine::State::EXEC);
	runnableCoQue_.push(co);
    load_++;
	DEBUG("add task <{}>, total task = {}", name, load_);
    if(epoll_.isEpolling()) {
        wakeupEpollCo();
    }
}

void Processor::addTask(const Coroutine::Callback& cb, const std::string& name) {
    if(idleCoQue_.empty()) {
        addTask(std::make_shared<Coroutine>(cb, name), name);
    }
    else {
        Coroutine::ptr co;

        co = idleCoQue_.front();
        idleCoQue_.pop();

        co->reset(cb);
        co->setName(name);
        addTask(co, name);
    }
}

void Processor::addPendingTask(const Coroutine::Callback& cb, const std::string& name) {
    pendingTasks_.emplace_back(cb, name);
    wakeupEpollCo();
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
	ssize_t n = ::write(wakeupFd_, &one, sizeof one);
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

void Processor::addPendingTasksIntoQueue() {
    std::vector<task> tasks;
    addingPendingTasks_ = true;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks.swap(pendingTasks_);
    }
    for(const auto& t : tasks) {
        addTask(t.first, t.second);
    }
    addingPendingTasks_ = false;
}



