#include "Util.h"
#include <string.h>
#include <execinfo.h>
#include <cxxabi.h>
#include "Log.h"
#include "Coroutine.h"


namespace burger {
namespace util {

thread_local pid_t t_cachedTid = 0;
static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");

thread_local char t_errnobuf[512];

const char* strerror_tl(int savedErrno) {
    return strerror_r(savedErrno, t_errnobuf, sizeof(t_errnobuf));
}

uint64_t getCoId() {
    return Coroutine::GetCoId();
}

static std::string demangle(const char* str) {
    size_t size = 0;
    int status = 0;
    std::string rt;
    rt.resize(256);
    if(1 == sscanf(str, "%*[^(]%*[^_]%255[^)+]", &rt[0])) {
        char* v = abi::__cxa_demangle(&rt[0], nullptr, &size, &status);
        if(v) {
            std::string result(v);
            free(v);
            return result;
        }
    }
    if(1 == sscanf(str, "%255s", &rt[0])) {
        return rt;
    }
    return str;
}

void Backtrace(std::vector<std::string>& bt, int size, int skip) {
    void** array = static_cast<void**>(malloc((sizeof(void*) * size)));
    int s = ::backtrace(array, size);

    char** strings = backtrace_symbols(array, s);
    if(strings == NULL) {
        ERROR("backtrace_synbols error");
        return;
    }
    for(int i = skip; i < s; ++i) {
        bt.push_back(demangle(strings[i]));
    }

    free(strings);
    free(array);
}

std::string BacktraceToString(int size, int skip, const std::string& prefix) {
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    std::stringstream ss;
    for(size_t i = 0; i < bt.size(); ++i) {
        ss << prefix << bt[i] << std::endl;
    }
    return ss.str();
}

} // namespace util

} // namespace burger



