#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <string>

// todo : string_view and python like string api

namespace burger {

class StringUtil {
public:
    static char* strToCharStar(const std::string & str);
};

} // namespace burger







#endif // STRINGUTIL_H