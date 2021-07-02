#include "burger/http/HttpMethod.h"
#include "burger/base/Util.h"

#include <iostream>
using namespace burger;
using namespace burger::http;

int main() {
    HttpMethod httpmethod = StringToHttpMethod("TRACE");
    // std::cout << httpmethod << std::endl;
    std::cout << util::as_integer(httpmethod) << std::endl;
    const char* s = "GET";
    HttpMethod httpmethod1 = CharsToHttpMethod(s);
    std::cout << util::as_integer(httpmethod1) << std::endl;  

    std::string res = HttpMethodToString(httpmethod1);
    std::cout << res << std::endl;
}