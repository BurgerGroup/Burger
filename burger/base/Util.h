#include <sstream>
#include <memory>
#include <thread>

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

} // namespace util

} // namespace burger
