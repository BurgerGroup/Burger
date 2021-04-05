#include "burger/http/HttpRequestParser.h"
#include "burger/http/HttpRequest.h"
#include "burger/base/StringUtil.h"
#include <iostream>
#include "burger/base/Log.h"

using namespace burger;
using namespace burger::http;

// const char test_request_data[] = "POST / HTTP/1.1\r\n"
//                         "Host: www.baidu.com\r\n"
//                         "Content-Length: 10\r\n\r\n"
//                         "1234567890";

const char test_request_data[] = "GET / HTTP/1.0\r\n"
                                "Host: localhost:9001\r\n"
                                "User-Agent: Python-urllib/1.17\r\n"
                                "Accept: */*\r\n\r\n";

int main() {
    HttpRequestParser parser;
    std::string tmp = test_request_data;
    // https://stackoverflow.com/questions/347949/how-to-convert-a-stdstring-to-const-char-or-char
    size_t s = parser.execute(&tmp[0], tmp.size());
    std::cout << "execute rt = " << s << " has_error  = " << parser.hasError()
        << " is_finished = " << parser.isFinished() << " total = " << tmp.size() 
        << " content_length = " << parser.getContentLength() << std::endl;

    tmp.resize(tmp.size() - s);
    std::cout << parser.getRequest()->toString() << std::endl;
    std::cout << tmp << std::endl;

}

