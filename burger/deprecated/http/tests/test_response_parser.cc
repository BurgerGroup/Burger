#include "burger/http/HttpResponseParser.h"
#include "burger/http/HttpResponse.h"
#include "burger/base/Log.h"

/*
telnet wwww.baidu.com 80

GET / HTTP/1.0

HTTP/1.1 200 OK
accept-ranges: bytes
cache-control: max-age=86400
connection: keep-alive
content-length: 81
content-type: text/html
date: Sat, 20 Mar 2021 05:48:53 GMT
etag: "51-47cf7e6ee8400"
expires: Sun, 21 Mar 2021 05:48:53 GMT
keep-alive: timeout=4
last-modified: Tue, 12 Jan 2010 13:48:00 GMT
proxy-connection: keep-alive
server: Apache

<html>
<meta http-equiv="refresh" content="0;url=http://www.baidu.com/">
</html>

*/

using namespace burger;
using namespace burger::http;

const char test_response_data[] = "HTTP/1.1 200 OK\r\n"
                                "accept-ranges: bytes\r\n"
                                "cache-control: max-age=86400\r\n"
                                "connection: keep-alive\r\n"
                                "content-length: 81\r\n"
                                "content-type: text/html\r\n"
                                "date: Sat, 20 Mar 2021 05:48:53 GMT\r\n"
                                "etag: \"51-47cf7e6ee8400\"\r\n"
                                "expires: Sun, 21 Mar 2021 05:48:53 GMT\r\n"
                                "keep-alive: timeout=4\r\n"
                                "last-modified: Tue, 12 Jan 2010 13:48:00 GMT\r\n"
                                "proxy-connection: keep-alive\r\n"
                                "server: Apache\r\n\r\n"
                                "<html>\r\n"
                                "<meta http-equiv=\"refresh\" content=\"0;url=http://www.baidu.com/\">\r\n"
                                "</html>\r\n";



int main() {
    LOGGER();
    HttpResponseParser parser;
    std::string tmp = test_response_data;
    size_t s = parser.execute(&tmp[0], tmp.size(), true);
    INFO("excute rt = {} has_error = {} is_finished = {} total = {} content_length = {} tmp[s] = ", 
            s, parser.hasError(), parser.isFinished(), tmp.size(), parser.getContentLength(), tmp[s]);

    tmp.resize(tmp.size() - s);
    INFO("{}", parser.getResponse()->toString());
    INFO("{}", tmp);

}
