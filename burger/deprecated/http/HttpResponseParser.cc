#include "HttpResponseParser.h"
#include "HttpResponse.h"


using namespace burger;
using namespace burger::http;

namespace {

void response_reason_cb(void *data, const char *at, size_t length) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    parser->getResponse()->setReason(std::string(at, length));
}

void response_status_cb(void *data, const char *at, size_t length) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    // todo : check
    std::cout << at << std::endl;
    // atoi 遇到空格停止
    HttpStatus status = static_cast<HttpStatus>(atoi(at));
    // HttpStatus status = static_cast<HttpStatus>(boost::lexical_cast<int>(at));
    parser->getResponse()->setStatus(status);
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
    parser->getResponse()->setVersion(version);
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
    parser->getResponse()->setHeader(std::string(field, flen)
                                ,std::string(value, vlen));
}

    
} // namespace

// todo same as request
uint64_t HttpResponseParser::s_response_buffer_size = 4 * 1024;
uint64_t HttpResponseParser::s_response_max_body_size = 64 * 1024 * 1024;

HttpResponseParser::HttpResponseParser() {
    response_ = std::make_shared<HttpResponse>();
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

size_t HttpResponseParser::execute(char* data, size_t len, bool chunck) {
    if(chunck) {
        httpclient_parser_init(&parser_);
    }
    size_t offset = httpclient_parser_execute(&parser_, data, len, 0);

    memmove(data, data + offset, (len - offset));
    return offset;
}

int HttpResponseParser::isFinished() { 
    return httpclient_parser_finish(&parser_); 
}

int HttpResponseParser::hasError() { 
    return error_ || httpclient_parser_has_error(&parser_); 
}

uint64_t HttpResponseParser::getContentLength() { 
    return response_->getHeaderAs<uint64_t>("content-length", 0); 
}

