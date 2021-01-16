#ifndef SINGLETON_H
#define SINGLETON_H

#include <boost/noncopyable.hpp>

// 能更方便点吗

namespace burger {

template<typename T>
class Singleton {
public:
    static T& Instance();
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
private:
    Singleton() = default;
    ~Singleton() = default;
};

template<typename T>
T& Singleton<T>::Instance() {
    static T OnlyInstance;
    return OnlyInstance;
}

template<typename T>
class SingletonPerThread {
public:
    static T& Instance();
    SingletonPerThread(const SingletonPerThread&) = delete;
    SingletonPerThread& operator=(const SingletonPerThread&) = delete;
private:
    SingletonPerThread() = default;
    ~SingletonPerThread() = default;
};

template<typename T>
T& SingletonPerThread<T>::Instance() {
    static thread_local T OnlyInstance;
    return OnlyInstance;
}


} // namespace burger




#endif // SINGLETON_H