#include "Processer.h"
#include "SocketsOps.h"
#include "CoEpoll.h"
#include "burger/base/Util.h"

using namespace burger;
using namespace burger::net;

static thread_local Processor* t_proc = nullptr;


Processor::Processor(Scheduler* scheduler) 
    : scheduler_(scheduler),
    epoll_(util::make_unique<CoEpoll>(this)),
    eventFd_(sockets::createEventfd()) {
	// 当有新事件来时唤醒Epoll 协程
    addTask([&](){
        while (!stop_) {
            if (comsumeWakeEvent() < 0) {
                ERROR("read eventfd : {}", strerror_tl(errno));
                break;
            }
        }
    }, "Wake");
    // GetCurCo(); todo need this?
}

void Processor::run() {
	if (GetProcesserOfThisThread() != nullptr) {
        CRITICAL("Run two processers in one thread");
	} else {
		GetProcesserOfThisThread() = this;
	}
	// setHookEnabled(true);
	Coroutine::ptr cur;

	//没有可以执行协程时调用epoll协程
	Coroutine::ptr epollCo = std::make_shared<Coroutine>(std::bind(&CoEpoll::wait, &epoll_, kPollTimeMs), "Epoll");

	while (!stop_) {
		{
            std::lock_guard<std::mutex> lock(mutex_);
			//没有协程时执行epoll协程
			if (coList_.empty()) {
				cur = epollCo;
				epoll_->setEpolling(true);
			} else {
                //  todo : 此处可优化
				for (auto it = coList_.begin();
				        it != coList_.end(); ++it) {
					cur = *it;
					coroutines_.erase(it);
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

void Processor::addTask(Coroutine::ptr co) {
    std::lock_guard<std::mutex> mutex_;
    coList_.push_back(co);
    load_++;
    if(epoll_->isEpolling()) {
        wakeupEpollCo();
    }
}

void Processor::addTask(const Coroutine::Callback& cb, std::string name) {
    addTask(std::make_shared<Coroutine>(Std::move(cb), name));
}

Processor* Processor::GetProcesserOfThisThread() {
    return t_proc;
}

