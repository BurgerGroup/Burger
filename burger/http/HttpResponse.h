#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <ostream>
#include <vector>
#include "HttpStatus.h"
#include "version.h"
#include "MapType.h"

namespace burger {
namespace http {

class HttpResponse {
public:
    using ptr = std::shared_ptr<HttpResponse>;
    HttpResponse(Version version = Version::kHttp11, bool close = true);

    HttpStatus getStatus() const { return status_;}
    Version getVersion() const { return version_;}
    const std::string& getBody() const { return body_;}
    const std::string& getReason() const { return reason_;}
    const MapType& getHeaders() const { return headers_;}
    // 如果存在返回对应值,否则返回def
    std::string getHeader(const std::string& key, const std::string& def = "") const;
    bool isClose() const { return close_;}
    
    void setStatus(HttpStatus status) { status_ = status;}
    void setVersion(Version version) { version_ = version;}
    void setBody(const std::string& body) { body_ = body;}
    void setReason(const std::string& reason) { reason_ = reason;}
    void setHeaders(const MapType& headers) { headers_ = headers;}
    void setClose(bool close) { close_ = close;}
    void setHeader(const std::string& key, const std::string& val);

    void delHeader(const std::string& key);

    // 如果存在且转换成功返回true,否则失败val=def
    template<class T>
    bool checkGetHeaderAs(const std::string& key, T& val, const T& def = T()) {
        return checkGetAs(m_headers, key, val, def);
    }
    // 如果存在且转换成功返回对应的值,否则返回def
    template<class T>
    T getHeaderAs(const std::string& key, const T& def = T()) {
        return getAs(m_headers, key, def);
    }
    // 序列化输出到流
    std::ostream& dump(std::ostream& os) const;
    std::string toString() const;

    void setRedirect(const std::string& uri);
    void setCookie(const std::string& key, const std::string& val,
                   time_t expired = 0, const std::string& path = "",
                   const std::string& domain = "", bool secure = false);
private:
    
    HttpStatus status_;  
    Version version_; 
    bool close_;   // 是否自动关闭
    std::string body_;
    std::string reason_;
    MapType headers_;
    std::vector<std::string> cookies_;


};

// 流式输出HttpResponse
std::ostream& operator<<(std::ostream& os, const HttpResponse& rsp);

} // namespace http

} // namespace burger



#endif // HTTPRESPONSE_H