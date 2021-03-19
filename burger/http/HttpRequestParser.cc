#include "HttpRequestParser.h"
#include "HttpRequest.h"
#include "burger/base/Log.h"

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
    parser->getData()->setMethod(m);
}

void request_uri_cb(void *data, const char *at, size_t length) {
}

void request_fragment_cb(void *data, const char *at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setFragment(std::string(at, length));
}

void request_path_cb(void *data, const char *at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setPath(std::string(at, length));
}

void request_query_cb(void *data, const char *at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setQuery(std::string(at, length));
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
    parser->getData()->setVersion(version);
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
    parser->getData()->setHeader(std::string(field, flen)
                                ,std::string(value, vlen));
}
    
} // namespace 


HttpRequestParser::HttpRequestParser()
    : error_(0) {
    data_ = std::make_shared<HttpRequest>();
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