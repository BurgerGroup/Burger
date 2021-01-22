#include <sstream>
#include <memory>
#include <thread>
#include <unistd.h>
#include <sys/syscall.h>

namespace burger {
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
    return static_cast<pid_t>(syscall(SYS_gettid));
}

} // namespace util

} // namespace burger
