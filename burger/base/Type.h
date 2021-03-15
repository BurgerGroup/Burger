#ifndef TYPE_H
#define TYPE_H

#include <string>

namespace burger {
class TypeUtil {
public:
    static int64_t strToInt64(const std::string& str);
    static int64_t strToInt64(const char* str);
    static double strToDouble(const std::string& str);
    static double strToDouble(const char* str);
}; 
} // namespace burger






#endif // TYPE_H