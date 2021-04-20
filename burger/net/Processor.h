#ifndef PROCESSER_H
#define PROCESSER_H

#include <boost/noncopyable.hpp>
#include <mutex>
#include <list>
#include "Coroutine.h"

namespace burger {
namespace net {

class Epoll;
class Scheduler;
class Processor : boost::noncopyable {
public:
	using ptr = std::shared_ptr<Processor>;

	Processor(Scheduler* scheduler);
	virtual ~Processor() {}

	virtual void run();
	void stop();
	bool stoped() { return stop_; }
	size_t getLoad() { return load_; }
	Scheduler* getScheduler() { return scheduler_; }
	void addTask(Coroutine::ptr co);
	void addTask(const Coroutine::Callback& cb, std::string name = "");
	// void updateEvent(int fd, int events, Coroutine::ptr coroutine = nullptr);
	// void removeEvent(int fd);

	static Processor* GetProcesserOfThisThread();

private:
	void wakeupEpollCo();
	ssize_t comsumeWakeEvent();

	bool stop_ = false;
	size_t load_ = 0;
    std::mutex mutex_;
	Scheduler* scheduler_;
    std::unique_ptr<Epoll> epoll_;

	int eventFd_;
	std::list<Coroutine::ptr> coList_;
};



} // namespace net

} // namespace burger



#endif // PROCESSER_H