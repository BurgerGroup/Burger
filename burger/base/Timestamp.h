/*
 * @Author: Shiyu Yi
 * @Github: https://github.com/chanchann
 */
#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include "copyable.h"
#include <iostream>
#include <boost/operators.hpp>
#include <chrono>
#include <string>
namespace burger {
using namespace std::chrono;
class Timestamp : public burger::copyable,    // 空基类；用于标识该类为可拷贝类型
                  public boost::less_than_comparable<Timestamp> {    // 只需要实现<，自动实现>,>=,<=
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);
    void swap(Timestamp& that);
    std::string toString() const;
    int64_t microSecondsSinceEpoch() const;
    static Timestamp now();   
    static const int kMicroSecondsPerSecond = 1000 * 1000;
private:
    int64_t microSecondsSinceEpoch_;
};


inline double timeDifference(Timestamp high, Timestamp low) {
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;    // 结果/（1000*1000），返回的是秒数
}

}; // namespace burger

#endif // TIMESTAMP_H