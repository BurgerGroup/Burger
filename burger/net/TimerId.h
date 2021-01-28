#ifndef TIMERID_H
#define TIMERID_H

#include <memory>
#include "burger/base/copyable.h"
#include "Timer.h"

namespace burger {
namespace net {

class Timer;

class TimerId : public burger::copyable {
public:
    TimerId();
    TimerId(std::shared_ptr<Timer> timer, int64_t seq);

    friend class TimerQueue;
private:
    std::shared_ptr<Timer> timer_;
    int64_t seq_;
};
} // namespace net

} // namespace burger



#endif // TIMERID_H