#include "Timestamp.h"

using namespace burger;

static_assert(sizeof(Timestamp) == sizeof(uint64_t),
            "Timestamp should be same size as uint64_t");

Timestamp::Timestamp() 
    : microSecondsSinceEpoch_(0) {
}

Timestamp::Timestamp(const Timestamp& that) 
    : microSecondsSinceEpoch_(that.microSecondsSinceEpoch_) {
}

Timestamp::Timestamp(uint64_t microSecondsSinceEpoch)
    : microSecondsSinceEpoch_(microSecondsSinceEpoch) {
}

Timestamp& Timestamp::operator=(const Timestamp& t) {
    this->microSecondsSinceEpoch_ = t.microSecondsSinceEpoch_;
    return *this;
}

void Timestamp::swap(Timestamp& that) {
    std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
}

std::string Timestamp::toString() const {
    return std::to_string(microSecondsSinceEpoch_ / kMicroSecondsPerSecond) 
        + "." + std::to_string(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);

}

// todo check accuracy
std::string Timestamp::toFormatTime() const {
    std::time_t time = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;  // ms --> s
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %X");
    return ss.str();
}

uint64_t Timestamp::microSecondsSinceEpoch() const {
    return microSecondsSinceEpoch_;
}

Timestamp Timestamp::now() {
    uint64_t timestamp = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
    return Timestamp(timestamp);
}









