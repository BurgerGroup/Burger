#ifndef RWMUTEX_H
#define RWMUTEX_H

#include <boost/noncopyable.hpp>

namespace burger {

// 局部读锁模板实现
// todo 深入研究下
// 模板类编写 复习
template<class T>
class ReadScopedLockImpl {
public:
    ReadScopedLockImpl(T& mutex)
        : mutex_(mutex) {
        mutex_.rdlock();
        locked_ = true;
    }
    ~ReadScopedLockImpl() {
        unlock();
    }
    void lock() {
        if(!locked_) {
            mutex_.rdlock();
            locked_ = true;
        }
    }
    void unlock() {
        if(locked_) {
            mutex_.unlock();
            locked_ = false;
        }
    }
private:
    T& mutex_;
    bool locked_;
};

// 局部写锁模板实现
template<class T>
class WriteScopedLockImpl {
public:
    WriteScopedLockImpl(T& mutex)
        : mutex_(mutex) {
        mutex_.wrlock();
        locked_ = true;
    }
    ~WriteScopedLockImpl() {
        unlock();
    }
    void lock() {
        if(!locked_) {
            mutex_.wrlock();
            locked_ = true;
        }
    }
    void unlock() {
        if(locked_) {
            mutex_.unlock();
            locked_ = false;
        }
    }
private:
    T& mutex_;
    bool locked_;
};

// 读写互斥量
class RWMutex : boost::noncopyable {
public:
    using ReadLock = ReadScopedLockImpl<RWMutex>; // 局部读锁
    using WriteLock = WriteScopedLockImpl<RWMutex> ; // 局部写锁
    RWMutex() {
        pthread_rwlock_init(&lock_, nullptr);
    }
    ~RWMutex() {
        pthread_rwlock_destroy(&lock_);
    }
    void rdlock() {
        pthread_rwlock_rdlock(&lock_);
    }
    void wrlock() {
        pthread_rwlock_wrlock(&lock_);
    }
    void unlock() {
        pthread_rwlock_unlock(&lock_);
    }
private:
    pthread_rwlock_t lock_; // 读写锁
};
 
} // namespace burger







#endif // RWMUTEX_H