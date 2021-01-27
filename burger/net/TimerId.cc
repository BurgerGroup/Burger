#include "TimerId.h"


using namespace burger;
using namespace burger::net;

TimerId::TimerId() :
    seq_(0) {
}

TimerId::TimerId(std::shared_ptr<Timer> timer, int64_t seq) :
    timer_(timer),
    seq_(seq) {
}




