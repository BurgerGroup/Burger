#ifndef HTTPREQUESTPARSER_H
#define HTTPREQUESTPARSER_H

#include "http11_parser.h"
#include <memory>

namespace burger {
namespace http {
    

class HttpRequest;
class HttpRequestParser {
public:
    using ptr = std::shared_ptr<HttpRequestParser>;
    HttpRequestParser();

    // 解析协议, 返回实际解析的长度,并且将已解析的数据移除
    size_t execute(char* data, size_t len);
    int isFinished();
    int hasError(); 
    std::shared_ptr<HttpRequest> getData() const { return data_;}
    void setError(int error) { error_ = error;}
    uint64_t getContentLength();
    const http_parser& getParser() const { return parser_;}
    // 返回HttpRequest协议解析的缓存大小
    static uint64_t GetHttpRequestBufferSize();
    static uint64_t GetHttpRequestMaxBodySize();
private:
    http_parser parser_;
    std::shared_ptr<HttpRequest> data_;
    int error_; // 1000: invalid method , 1001: invalid version, 1002: invalid field
};


} // namespace http

} // namespace burge 




#endif // HTTPREQUESTPARSER_H