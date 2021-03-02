#include "Util.h"
#include <string.h>

namespace burger {
namespace util {
thread_local pid_t t_cachedTid = 0;
static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");

thread_local char t_errnobuf[512];

const char* strerror_tl(int savedErrno) {
    return strerror_r(savedErrno, t_errnobuf, sizeof(t_errnobuf));
}

} // namespace util

} // namespace burger



