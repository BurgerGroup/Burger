  
// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

// Taken from Muduo and modified

#ifndef TIMERID_H
#define TIMERID_H

#include <memory>
#include "burger/base/copyable.h"

namespace burger {
namespace net {
class Timer;
class TimerQueue;
class CoTimerQueue;
class Scheduler;
class TimerId : public burger::copyable {
public:
    TimerId();
    TimerId(std::shared_ptr<Timer> timer, uint64_t seq);
    // tips : 友元只需要声明不需要include
    friend class TimerQueue;
    friend class CoTimerQueue;
    friend class Scheduler;
private:
    std::shared_ptr<Timer> timer_;
    uint64_t seq_;
};
} // namespace net

} // namespace burger



#endif // TIMERID_H