#include "HttpResponseParser.h"
#include "HttpResponse.h"
#include "burger/base/Log.h"

using namespace burger;
using namespace burger::http;

namespace {

void response_reason_cb(void *data, const char *at, size_t length) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    parser->getData()->setReason(std::string(at, length));
}

void response_status_cb(void *data, const char *at, size_t length) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    // todo : check
    HttpStatus status = static_cast<HttpStatus>(boost::lexical_cast<int>(at));
    // HttpStatus status = (HttpStatus)(atoi(at));
    parser->getData()->setStatus(status);
}

void response_chunk_cb(void *data, const char *at, size_t length) {
}

void response_version_cb(void *data, const char *at, size_t length) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    Version version;
    if(strncmp(at, "HTTP/1.1", length) == 0) {
        version = Version::kHttp11;
    } else if(strncmp(at, "HTTP/1.0", length) == 0) {
        version = Version::kHttp10;
    } else {
        WARN("invalid http response version: {}", std::string(at, length));
        parser->setError(1001);
        return;
    }
    parser->getData()->setVersion(version);
}

void response_header_done_cb(void *data, const char *at, size_t length) {
}

void response_last_chunk_cb(void *data, const char *at, size_t length) {
}

void response_http_field_cb(void *data, const char *field, size_t flen
                           ,const char *value, size_t vlen) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    if(flen == 0) {
        WARN("invalid http response field length == 0");
        //parser->setError(1002);
        return;
    }
    parser->getData()->setHeader(std::string(field, flen)
                                ,std::string(value, vlen));
}

    
} // namespace

HttpResponseParser::HttpResponseParser() {
    data_ = std::make_shared<HttpResponse>();
    httpclient_parser_init(&parser_);
    parser_.reason_phrase = response_reason_cb;
    parser_.status_code = response_status_cb;
    parser_.chunk_size = response_chunk_cb;
    parser_.http_version = response_version_cb;
    parser_.header_done = response_header_done_cb;
    parser_.last_chunk = response_last_chunk_cb;
    parser_.http_field = response_http_field_cb;
    parser_.data = this;
}

