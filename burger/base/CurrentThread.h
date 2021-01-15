#ifndef CURRENTTHREAD_H
#define CURRENTTHREAD_H

#include <cxxabi.h>
#include <execinfo.h>
#include <stdlib.h>
#include <string>

namespace burger {

namespace CurrentThread {

extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char* t_threadName;
void cacheTid();

inline int tid() {
    if (__builtin_expect(t_cachedTid == 0, 0)) {
        cacheTid();
    }
    return t_cachedTid;
}

inline const char* tidString() {// for logging
    return t_tidString;
}

inline int tidStringLength() {// for logging
    return t_tidStringLength;
}

inline const char* name() {
    return t_threadName;
}

bool isMainThread();

void sleepUsec(int64_t usec);  // for testing

std::string stackTrace(bool demangle);
}  // namespace CurrentThread
 
} // namespace burger

#endif // CURRENTTHREAD_H