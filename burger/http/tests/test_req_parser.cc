#include "burger/http/HttpRequestParser.h"
#include "burger/http/HttpRequest.h"
#include "burger/base/StringUtil.h"
#include <iostream>
#include "burger/base/Log.h"

using namespace burger;
using namespace burger::http;

const char test_request_data[] = "POST / HTTP/1.1\r\n"
                        "Host: www.baidu.com\r\n"
                        "Content-Length: 10\r\n\r\n"
                        "1234567890";
int main() {
    LOGGER;
    HttpRequestParser parser;
    std::string tmp = test_request_data;
    // https://stackoverflow.com/questions/347949/how-to-convert-a-stdstring-to-const-char-or-char
    size_t s = parser.execute(&tmp[0], tmp.size());
    INFO("execute rt = {} has_error  = {} is_finished= {}, total = {}, content_length = {}",
        s, parser.hasError(), parser.isFinished(), tmp.size(), parser.getContentLength());
    tmp.resize(tmp.size() - s);
    INFO("{}", parser.getRequest()->toString());
    INFO("{}", tmp);
}

