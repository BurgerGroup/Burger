#include "HttpRequestParser.h"
#include "HttpRequest.h"


using namespace burger; 
using namespace burger::http; 

namespace  {

void request_method_cb(void *data, const char *at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    HttpMethod m = CharsToHttpMethod(at);

    if(m == HttpMethod::INVALID_METHOD) {
        WARN("invalid http request method: {}", std::string(at, length));
        parser->setError(1000);
        return;
    }
    parser->getRequest()->setMethod(m);
}

void request_uri_cb(void *data, const char *at, size_t length) {
}

void request_fragment_cb(void *data, const char *at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getRequest()->setFragment(std::string(at, length));
}

void request_path_cb(void *data, const char *at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getRequest()->setPath(std::string(at, length));
}

void request_query_cb(void *data, const char *at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getRequest()->setQuery(std::string(at, length));
}

void request_version_cb(void *data, const char *at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    Version version;
    if(strncmp(at, "HTTP/1.1", length) == 0) {
        version = Version::kHttp11;
    } else if(strncmp(at, "HTTP/1.0", length) == 0) {
        version = Version::kHttp10;
    } else {
        WARN("invalid http request version: {}", std::string(at, length));
        parser->setError(1001);
        return;
    }
    parser->getRequest()->setVersion(version);
}

void request_header_done_cb(void *data, const char *at, size_t length) {
    //HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
}

void request_http_field_cb(void *data, const char *field, size_t flen
                           ,const char *value, size_t vlen) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    if(flen == 0) {
        WARN("invalid http request field length == 0");
        //parser->setError(1002);
        return;
    }
    parser->getRequest()->setHeader(std::string(field, flen)
                                ,std::string(value, vlen));
}

} // namespace 

// todo config 
// todo requestSizeInitializer
// 防止恶意请求
uint64_t HttpRequestParser::s_request_buffer_size = 4 * 1024;
uint64_t HttpRequestParser::s_request_max_body_size = 64 * 1024 * 1024;



HttpRequestParser::HttpRequestParser()
    : error_(0) {
    request_ = std::make_shared<HttpRequest>();
    http_parser_init(&parser_);
    parser_.request_method = request_method_cb;
    parser_.request_uri = request_uri_cb;
    parser_.fragment = request_fragment_cb;
    parser_.request_path = request_path_cb;
    parser_.query_string = request_query_cb;
    parser_.http_version = request_version_cb;
    parser_.header_done = request_header_done_cb;
    parser_.http_field = request_http_field_cb;
    parser_.data = this;
    
}

// todo : 增加个off能优化吗
// @ 1: 成功  @ -1: 有错误  @ >0: 已处理的字节数，且data有效数据为len - v;
size_t HttpRequestParser::execute(char* data, size_t len) {
    size_t offset = http_parser_execute(&parser_, data, len, 0);
    // if(offset == -1) {
    //     WARN("invalid request"); 
    // }
    // memmove(data, data + offset, (len - offset));
    return offset;
}

int HttpRequestParser::isFinished() { 
    return http_parser_finish(&parser_); 
}

int HttpRequestParser::hasError() { 
    return error_ || http_parser_has_error(&parser_); 
}

uint64_t HttpRequestParser::getContentLength() { 
    return request_->getHeaderAs<uint64_t>("content-length", 0); 
}


