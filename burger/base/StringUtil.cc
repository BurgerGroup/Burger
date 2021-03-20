#include "StringUtil.h"

using namespace burger;

char* StringUtil::strToCharStar(const std::string & str) {
    return const_cast<char*>(str.c_str());
}
