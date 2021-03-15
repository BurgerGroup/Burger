#include "Type.h"

using namespace burger;

int64_t TypeUtil::strToInt64(const std::string& str) {
    if(str.empty()) {
        return 0;
    }
    return strtoull(str.c_str(), nullptr, 10);    
}

int64_t TypeUtil::strToInt64(const char* str) {
    if(str == nullptr) {
        return 0;
    }
    return strtoull(str, nullptr, 10);
}

double TypeUtil::strToDouble(const std::string& str) {
    if(str.empty()) {
        return 0;
    }
    return atof(str.c_str());
}

double TypeUtil::strToDouble(const char* str) {
    if(str == nullptr) {
        return 0;
    }
    return atof(str);
}
