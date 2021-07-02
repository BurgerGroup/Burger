#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <ostream>
#include <vector>
#include <cstring>
#include "HttpStatus.h"
#include "Version.h"
#include "MapType.h"

namespace burger {
namespace http {

class HttpResponse {
public:
    using ptr = std::shared_ptr<HttpResponse>;
    HttpResponse(bool close = true, Version version = Version::kHttp11);

    HttpStatus getStatus() const { return status_;}
    Version getVersion() const { return version_;}
    const std::string& getBody() const { return body_;}
    const std::string& getReason() const { return reason_;}
    const MapType& getHeadersMap() const { return headersMap_;}
    // 如果存在返回对应值,否则返回def
    std::string getHeader(const std::string& key, const std::string& def = "") const;
    bool isClose() const { return close_;}
    
    void setStatus(HttpStatus status) { status_ = status;}
    void setVersion(Version version) { version_ = version;}
    void setBody(const std::string& body) { body_ = body;}
    void setReason(const std::string& reason) { reason_ = reason;}
    void setHeadersMap(const MapType& headersMap) { headersMap_ = headersMap;}
    void setClose(bool close) { close_ = close;}
    void setHeader(const std::string& key, const std::string& val);

    void delHeader(const std::string& key);


    // template<class T>
    // bool checkGetHeaderAs(const std::string& key, T& val, const T& def = T());

    // 如果存在且转换成功返回对应的值,否则返回def
    template<class T>
    T getHeaderAs(const std::string& key, const T& def = T()) {
        return getAs(headersMap_, key, def);
    }

    // 序列化输出到流
    std::ostream& dump(std::ostream& os) const;
    std::string toString() const;

    void setRedirect(const std::string& uri);
    // void setCookie(const std::string& key, const std::string& val,
                //    time_t expired = 0, const std::string& path = "",
                //    const std::string& domain = "", bool secure = false);
private:
    HttpStatus status_;  
    bool close_;   // 是否自动关闭
    Version version_; 
    std::string body_;
    std::string reason_;
    MapType headersMap_;
    std::vector<std::string> cookies_;


};

// 流式输出HttpResponse
// std::ostream& operator<<(std::ostream& os, const HttpResponse& rsp);

// 如果存在且转换成功返回true,否则失败val=def
// template<class T>
// bool HttpResponse::checkGetHeaderAs(const std::string& key, T& val, const T& def = T()) {
//     return checkGetAs(headers_, key, val, def);
// }


} // namespace http

} // namespace burger



#endif // HTTPRESPONSE_H