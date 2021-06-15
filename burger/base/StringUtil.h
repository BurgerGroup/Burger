#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <string>
#include <vector>

#define MAX_32BIT_INT 2147483647

// ref : https://github.com/imageworks/pystring/blob/master/pystring.h
// todo : string_view and python like string api

namespace burger {

class StringUtil {
public:
    static char* strToCharStar(const std::string & str);
    
    static void split(const std::string& str, std::vector<std::string>& result, const std::string& sep = "", int maxsplit = -1);
    static bool isdigit(const std::string& str);
private:
    static void split_whitespace(const std::string& str, std::vector<std::string>& result, int maxsplit);
};

} // namespace burger







#endif // STRINGUTIL_H