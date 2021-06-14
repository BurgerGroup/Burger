#include "Processor.h"
#include "SocketsOps.h"
#include "burger/base/Util.h"
#include "burger/base/Log.h"
#include "burger/base/Config.h"
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
    : scheduler_(scheduler),
    epoll_(this),
    timerQueue_(util::make_unique<CoTimerQueue>(this)),
    wakeupFd_(sockets::createEventfd()),
    threadId_(util::tid()) {
    TRACE("Processor created {}", fmt::ptr(this));
    if (t_proc) {
    	CRITICAL("Another Processor {} exists in this Thread( tid = {} ) ...", fmt::ptr(t_proc), util::tid()); 
	} else {
		t_proc = this;
	}
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
    // 预先创建co存入idelQue中
    size_t preCoNum = Config::Instance().getSize("coroutine", "preCoNum", 1000);
	for(size_t i = 0; i < preCoNum; i++) {
        idleCoQue_.emplace(std::make_shared<Coroutine>(nullptr));
    }
    // ERROR("Pre coroutine number : {}", preCoNum);
    INFO("Pre coroutine number : {}", preCoNum);
}

// https://zhuanlan.zhihu.com/p/321947743
Processor::~Processor() {
	::close(wakeupFd_);
	TRACE("Processor {} dtor", fmt::ptr(this));
    t_proc = nullptr;
};

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
            cur = std::move(runnableCoQue_.front());
            runnableCoQue_.pop();
        } 
		cur->resume();
		if (cur->getState() == Coroutine::State::TERM) {
            --load_;
            idleCoQue_.push(cur);
            int fd = cur->getFd();
            if(fd != -1) {
                epoll_.removeEvent(fd);
            } 
            if(connMap_.count(fd)) {
                connMap_.erase(fd);
            }
		}
        // 避免在其他线程添加任务时错误地创建多余的协程（确保协程只在processor中）
        addPendingTasksIntoQueue();
	}
    TRACE("Processor {} stop running", fmt::ptr(this));
	epollCo->resume();  // epoll进去把cb执行完
}

void Processor::stop() {
	stop_ = true; 
    // todo : 对比和判断isInLoopThread
	if(epoll_.isEpolling()) {
		wakeupEpollCo();
	}
}

Coroutine::ptr Processor::resetAndGetCo(const Coroutine::Callback& cb, const std::string& name) {
        if(idleCoQue_.empty()) {
            // ++totolCoCreateTimes_; // for test
            return std::make_shared<Coroutine>(cb, name);
        } else {
            Coroutine::ptr co = idleCoQue_.front();
            idleCoQue_.pop();

            co->reset(cb);
            co->setName(name);
            return co;
        }
}

void Processor::addTask(Coroutine::ptr co) {
    co->setState(Coroutine::State::EXEC);
    runnableCoQue_.push(co);
    load_++;
	DEBUG("add task <{}>, total task = {}", co->getName(), load_);
    // if(epoll_.isEpolling()) {
    //     wakeupEpollCo();
    // }
}

void Processor::addTask(const Coroutine::Callback& cb, const std::string& name) {
    if(isInProcThread()) 
        addTask(resetAndGetCo(cb, name));
    else 
        addPendingTask(cb, name);
}

void Processor::addPendingTask(const Coroutine::Callback& cb, const std::string& name) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingTasks_.emplace_back(cb, name);
    }
    if(epoll_.isEpolling())
        wakeupEpollCo();
}

void Processor::updateEvent(int fd, int events) {
	epoll_.updateEvent(fd, events);
}

void Processor::removeEvent(int fd) {
	epoll_.removeEvent(fd);
}

Processor* Processor::GetProcesserOfThisThread() {
    return t_proc;
}

void Processor::assertInProcThread() {
    if(!isInProcThread()) {
        abortNotInProcThread();
    }
}

bool Processor::isInProcThread() const {
    return threadId_ == util::tid();
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

// 保存conn，延长conn的生命周期
void Processor::addToConnMap(int fd, std::shared_ptr<CoTcpConnection> conn) {
    connMap_[fd] = conn;  // fixme need to check?
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

void Processor::abortNotInProcThread() {
    CRITICAL("Processor::abortNotInLoopThread - processor {} was \
        created in threadId_ = {}, and current id = {} ...", 
        fmt::ptr(this), threadId_, util::tid());
}


