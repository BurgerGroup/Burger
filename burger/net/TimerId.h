#ifndef TIMERID_H
#define TIMERID_H

#include <memory>
#include "burger/base/copyable.h"

namespace burger {
namespace net {
class Timer;
class TimerQueue;
class TimerId : public burger::copyable {
public:
    TimerId();
    TimerId(std::shared_ptr<Timer> timer, uint64_t seq);
    // tips : 友元只需要声明不需要include
    friend class TimerQueue;
private:
    std::shared_ptr<Timer> timer_;
    uint64_t seq_;
};
} // namespace net

} // namespace burger



#endif // TIMERID_H