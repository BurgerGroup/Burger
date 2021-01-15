/*
 * @Author: Shiyu Yi
 * @Github: https://github.com/chanchann
 */
#include "Timestamp.h"

using namespace burger;

static_assert(sizeof(Timestamp) == sizeof(int64_t),
            "Timestamp should be same size as int64_t");

Timestamp::Timestamp() : microSecondsSinceEpoch_(0) 
{}

Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
        : microSecondsSinceEpoch_(microSecondsSinceEpoch) 
{}

void Timestamp::swap(Timestamp& that) {
    std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
}

std::string Timestamp::toString() const {
    return std::to_string(microSecondsSinceEpoch_ / kMicroSecondsPerSecond) 
        + "." + std::to_string(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);

}

int64_t Timestamp::microSecondsSinceEpoch() const {
    return microSecondsSinceEpoch_;
}

Timestamp Timestamp::now() {
    int64_t timestamp = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
    return Timestamp(timestamp);
}









