#ifndef HTTPRESPONSEPARSER_H
#define HTTPRESPONSEPARSER_H

#include <memory>
#include <boost/lexical_cast.hpp>  
#include "httpclient_parser.h"
#include "burger/base/Log.h"
namespace burger {
namespace http {

class HttpResponse;
class HttpResponseParser {
public:
    using ptr = std::shared_ptr<HttpResponseParser>;

    HttpResponseParser();
    // 解析HTTP响应协议, chunck 是否在解析chunck, 返回实际解析的长度,并且移除已解析的数据
    // http 不一定能马上出来，可能多个包才能拼完整
    size_t execute(char* data, size_t len, bool chunck);
    int isFinished();
    int hasError();
    std::shared_ptr<HttpResponse> getResponse() const { return response_; }
    void setError(int error) { error_ = error;}
    uint64_t getContentLength();
    const httpclient_parser& getParser() const { return parser_;}
public:
    static uint64_t GetHttpResponseBufferSize() { return s_response_buffer_size; }
    static uint64_t GetHttpResponseMaxBodySize() { return s_response_max_body_size; }
private:
    httpclient_parser parser_;
    std::shared_ptr<HttpResponse> response_;
    int error_;  // 1001: invalid version,  1002: invalid field
    static uint64_t s_response_buffer_size;
    static uint64_t s_response_max_body_size;
};
    
} // namespace http

} // namespace burger


#endif // HTTPRESPONSEPARSER_H