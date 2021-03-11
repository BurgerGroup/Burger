#ifndef UTIL_H
#define UTIL_H

#include <sstream>
#include <memory>
#include <thread>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <boost/lexical_cast.hpp>

namespace burger {

template<typename To, typename From>
inline To implicit_cast(From const &f) {
    return f;
}

namespace util {
/**
 * @brief Convert std::thread::id to std::string
 */
inline std::string threadIdToStr(const std::thread::id id) {
    std::stringstream ss;
    ss << id;
    return ss.str();
}

/**
 * @brief C++11 version make_unique<T>()
 */
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/**
 * @brief Get tid (Kernel)
 */
inline pid_t gettid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

extern thread_local pid_t t_cachedTid;

inline void cacheTid() {
    if (t_cachedTid == 0) {
        t_cachedTid = gettid();
    }
}

inline pid_t tid() {
    // todo
    if (__builtin_expect(t_cachedTid == 0, 0)) {
        cacheTid();
    }
    return t_cachedTid;
}

const char* strerror_tl(int savedErrno);

// 忽略大小写比较仿函数
struct CaseInsensitiveLess {
    bool operator()(const std::string& lhs, const std::string& rhs) const;
};


template<class V, class Map, class K>
V GetParamValue(const Map& m, const K& k, const V& def = V()) {
    auto it = m.find(k);
    if(it == m.end()) {
        return def;
    }
    try {
        return boost::lexical_cast<V>(it->second);
    } catch (...) {
        // 虽然在c中可是使用类似于atoi之类的函数对字符串转换成整型，但是我们在这儿还是推荐使用这个函数
        // 如果转换发生了错误，lexical_cast会抛出一个bad_lexical_cast异常，因此程序中需要对其进行捕捉。
    }
    return def;
}

} // namespace util

} // namespace burger

#endif // UTIL_H
