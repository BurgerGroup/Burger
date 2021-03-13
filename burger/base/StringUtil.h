#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <string>

namespace burger {

class StringUtil {
public:
    static std::string Format(const char* fmt, ...);
    static std::string Formatv(const char* fmt, va_list ap);
};

} // namespace burger







#endif // STRINGUTIL_H