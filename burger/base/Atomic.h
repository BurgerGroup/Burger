#ifndef ATOMIC_H
#define ATOMIC_H

#include <boost/noncopyable.hpp>

namespace burger {

namespace detail {

template<typename T>  
class AtomicIntegerT : boost::noncopyable {
public:
    AtomicIntegerT() : value_(0)
    {}

    T get() {
        return __sync_val_compare_and_swap(&value_, 0, 0); // 原子操作：获取value_（等于0设置为0，也是获取value_）
    }

    T getAndAdd(T x) {
        return __sync_fetch_and_add(&value_, x);    // 原子操作：获取value_，然后加上给定的X
    }

    T addAndGet(T x) {
        return getAndAdd(x) + x;   
    }

    T incrementAndGet() {
        return addAndGet(1);
    }

    T decrementAndGet() {
        return addAndGet(-1);
    }

    void add(T x) {
        getAndAdd(x);
    }

    void increment() {
        incrementAndGet();
    }

    void decrement() {
        decrementAndGet();
    }

    T getAndSet(T newValue) {
        return __sync_lock_test_and_set(&value_, newValue); // 原子操作：获取value_，并且设置为新的value_
    }

private:
    volatile T value_;    // 避免编译器优化,保证每次取值都是直接从内存中取值
};

} // namespace detail
/* 设置常用的类型 */

using AtomicInt32 = detail::AtomicIntegerT<int32_t>;  
using AtomicInt64 = detail::AtomicIntegerT<int64_t> ;
} // namespace burger



#endif // ATOMIC_H