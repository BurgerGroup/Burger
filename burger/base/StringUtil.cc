#include "StringUtil.h"

using namespace burger;

char* StringUtil::strToCharStar(const std::string & str) {
    return const_cast<char*>(str.c_str());
}

// sep 是分割符，默认是空格分割
// maxsplit 代表最大分割次数，-1是
void StringUtil::split(const std::string& str, std::vector<std::string>& result, const std::string& sep, int maxsplit) {
    result.clear();
    if(maxsplit < 0) maxsplit = MAX_32BIT_INT;//result.max_size();
    if (sep.size() == 0) {
        split_whitespace(str, result, maxsplit);
        return;
    }
    std::string::size_type i,j, len = str.size(), n = sep.size();
    i = j = 0;
    while (i+n <= len) {
        if (str[i] == sep[0] && str.substr( i, n ) == sep) {
            if (maxsplit-- <= 0) break;
            result.push_back( str.substr(j, i - j));
            i = j = i + n;
        } else {
            i++;
        }
    }

    result.push_back( str.substr( j, len-j ) );
}

bool StringUtil::isdigit(const std::string& str) {
    std::string::size_type len = str.size(), i;
    if(len == 0) return false;
    if(len == 1) return ::isdigit(str[0]);

    for(i = 0; i < len; ++i) {
        if (! ::isdigit(str[i])) return false;
    }
    return true;
}

void StringUtil::split_whitespace(const std::string& str, std::vector<std::string>& result, int maxsplit) {
    std::string::size_type i, j, len = str.size();
    for (i = j = 0; i < len; ) {
        while(i < len && ::isspace(str[i])) i++;
        // 获取一轮第一个不为空格的地方
        j = i;
        // 获取到第二个为空格的地方
        // 中间就是一个单词
        while(i < len && ! ::isspace(str[i])) i++;  

        if (j < i) {
            if (maxsplit-- <= 0) break;
            result.push_back(str.substr(j, i - j));
            // 防止最后一个空串
            while(i < len && ::isspace(str[i])) i++;
            j = i;
        }
    }
    if (j < len) {
        result.push_back(str.substr(j, len - j));
    }
}
