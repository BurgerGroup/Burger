#include "HttpRequest.h"

using namespace burger;
using namespace burger::http;

HttpRequest::HttpRequest(Version version, bool close)
    : method_(HttpMethod::GET),
    version_(version),
    close_(close),
    parserParamFlag_(0),
    path_("/") {
}

std::string HttpRequest::getHeader(const std::string& key, const std::string& def) const {
    auto it = headersMap_.find(key);
    return it == headersMap_.end() ? def : it->second;
}



/**
 * GET /uri HTTP/1.1
 * Host: www.burger.com
*/
std::ostream& HttpRequest::dump(std::ostream& os) const {
    os << HttpMethodToString(method_) << " "
       << path_
       << (query_.empty() ? "" : "?")
       << query_
       << (fragment_.empty() ? "" : "#")
       << fragment_
       << " "
       << httpVersionTostr(version_)
       << "\r\n";
    // websocket todo 
    os << "connection: " << (close_ ? "close" : "keep-alive") << "\r\n";
    for(auto& header : headersMap_) {
        if(strcasecmp(header.first.c_str(), "connection") == 0) {
            continue;
        }
        os << header.first << ": " << header.second << "\r\n";
    }
    if(!body_.empty()) {
        os << "content-length: " << body_.size() << "\r\n\r\n"
           << body_;
    } else {
        os << "\r\n";
    }
    return os;
}

std::string HttpRequest::toString() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

// void HttpRequest::initBodyParam()  {
//     if(m_parserParamFlag & 0x2) {
//         return;
//     }
//     std::string content_type = getHeader("content-type");
//     if(strcasestr(content_type.c_str(), "application/x-www-form-urlencoded") == nullptr) {
//         m_parserParamFlag |= 0x2;
//         return;
//     }
//     PARSE_PARAM(m_body, m_params, '&',);
//     m_parserParamFlag |= 0x2;
// }
