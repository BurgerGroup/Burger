#include "HttpResponse.h"

using namespace burger;
using namespace burger::http;

HttpResponse::HttpResponse(bool close, Version version) 
    : status_(HttpStatus::OK),
    close_(close),
    version_(version) {
}

std::string HttpResponse::getHeader(const std::string& key, const std::string& def) const {
    auto it = headersMap_.find(key);
    return it == headersMap_.end() ? def : it->second;    
}

void HttpResponse::setHeader(const std::string& key, const std::string& val) {
    headersMap_[key] = val;
}

void HttpResponse::delHeader(const std::string& key)  {
    headersMap_.erase(key);
}

// HTTP/1.1 200 OK
std::ostream& HttpResponse::dump(std::ostream& os) const {
    os << httpVersionTostr(version_) 
        << " "
        << util::as_integer(status_)
        << " "
        << (reason_.empty() ? HttpStatusToString(status_) : reason_)
        << "\r\n";
    for(auto& header : headersMap_) {
        if(strcasecmp(header.first.c_str(), "connection") == 0) {
            continue;
        }
        os << header.first << ": " << header.second << "\r\n";
    }
    os << "connection: " << (close_ ? "close" : "keep-alive") << "\r\n";
    if(!body_.empty()) {
        os << "content-length: " << body_.size() << "\r\n\r\n"
            << body_;
    } else {
        os << "\r\n";
    }
    return os;
}


std::string HttpResponse::toString() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

void HttpResponse::setRedirect(const std::string& uri) {
    status_ = HttpStatus::FOUND;
    setHeader("Location", uri);
}




