#include "Scheduler.h"
#include "burger/base/Log.h"
#include "burger/base/Util.h"

namespace {
#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe {
public:
    IgnoreSigPipe() {
        ::signal(SIGPIPE, SIG_IGN);
        TRACE("Ignore SIGPIPE");
    }
};
#pragma GCC diagnostic error "-Wold-style-cast"
IgnoreSigPipe initObj;

}  // namespace

using namespace burger;
using namespace burger::net;

Scheduler::Scheduler(size_t threadNum) 
    : threadNum_(threadNum),
    mainProc_(this),
    timerQueue_(util::make_unique<TimerQueue >()) {
    assert(threadNum_ > 0);
    // assert(Processer::GetProcesserOfThisThread() == nullptr);
    workProc_.push_back(&mainProc_);
}

Scheduler::~Scheduler() {
    stop();
}



