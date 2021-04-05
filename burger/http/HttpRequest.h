#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "burger/base/copyable.h"
#include "burger/base/Timestamp.h"
#include "burger/base/Util.h"
#include "HttpMethod.h"
#include "MapType.h"
#include "Version.h"
#include "http11_parser.h"
#include <map>
#include <memory>
#include <ostream>

// uri : http://www.xxx.com:80/page/xxx?id=110&v=20#fr
// todo : websocket 
namespace burger {
namespace http {

class HttpResponse;
class HttpRequest : public burger::copyable {
public:
    using ptr = std::shared_ptr<HttpRequest>;
    HttpRequest(Version version = Version::kHttp11, bool close = true);
    // std::shared_ptr<HttpResponse> createResponse();
    
    HttpMethod getMethod() const { return method_;}
    Version getVersion() const { return version_; }
    const std::string& getPath() { return path_; }
    const std::string& getQuery() const { return query_; }
    const std::string& getBody() const { return body_; }
    const MapType& getHeadersMap() const { return headersMap_; }
    const MapType& getParamsMap() const { return paramsMap_; }
    const MapType& getCookiesMap() const { return cookiesMap_; }
    // 如果关键字key存在则返回对应值,否则返回默认值def
    std::string getHeader(const std::string& key, const std::string& def = "") const;
    // std::string getParam(const std::string& key, const std::string& def = "");
    // std::string getCookie(const std::string& key, const std::string& def = "");

    void setMethod(HttpMethod method) { method_ = method;}
    void setVersion(Version version) { version_ = version;}
    void setPath(const std::string& path) { path_ = path; }
    void setQuery(const std::string& query) { query_ = query; }
    void setFragment(const std::string& fragment) { fragment_ = fragment;}
    void setBody(const std::string& body) { body_ = body;}
    void setClose(bool close) { close_ = close; }
    void setHeadersMap(const MapType& headers) { headersMap_ = headers;}
    void setParamsMap(const MapType& params) { paramsMap_ = params;}
    void setCookiesMap(const MapType& cookies) { cookiesMap_ = cookies;}

    void setHeader(const std::string& key, const std::string& val) { headersMap_[key] = val; }
    void setParam(const std::string& key, const std::string& val) { paramsMap_[key] = val; }
    void setCookie(const std::string& key, const std::string& val) { cookiesMap_[key] = val; }
    
    // void delHeader(const std::string& key);
    // void delParam(const std::string& key);
    // void delCookie(const std::string& key);
    
    // val 如果存在,val非空则赋值
    // todo why ptr?
    // bool hasHeader(const std::string& key, std::string* val = nullptr);
    // bool hasParam(const std::string& key, std::string* val = nullptr);
    // bool hasCookie(const std::string& key, std::string* val = nullptr);
    bool isClose() const { return close_; }

    // template<class T>
    // bool checkGetHeaderAs(const std::string& key, T& val, const T& def = T());

    // 如果存在且转换成功返回对应的值,否则返回def
    template<class T>
    T getHeaderAs(const std::string& key, const T& def = T()) {
        return getAs(headersMap_, key, def);
    }

    // template<class T>
    // bool checkGetParamAs(const std::string& key, T& val, const T& def = T());

    // template<class T>
    // T getParamAs(const std::string& key, const T& def = T());

    // template<class T>
    // bool checkGetCookieAs(const std::string& key, T& val, const T& def = T());

    // template<class T>
    // T getCookieAs(const std::string& key, const T& def = T());

    // 序列化输出到流中
    std::ostream& dump(std::ostream& os) const;
    std::string toString() const;
    // void init();
    // void initParam();
    // void initQueryParam();
    // void initBodyParam();
    // void initCookies();
private:    
    HttpMethod method_;   // 请求方法
    Version version_;  // http 协议版本
    bool close_;   // 是否自动关闭
    uint8_t parserParamFlag_;  // todo
    std::string path_;   // 请求路径
    std::string query_;  // 请求参数
    std::string fragment_;  // todo 请求fragment  
    std::string body_;  // 请求消息体
    Timestamp receiveTime_;  // 请求时间
    MapType headersMap_;    // 请求头部列表MAP
    MapType paramsMap_;   // 请求参数MAP
    MapType cookiesMap_;  // 请求Cookie MAP
};
// 流式输出HttpRequest
// std::ostream& operator<<(std::ostream& os, const HttpRequest& req);


// // 如果存在且转换成功返回true,否则失败val=def
// template<class T>
// bool HttpRequest::checkGetHeaderAs(const std::string& key, T& val, const T& def = T()) {
//     return checkGetAs(headersMap_, key, val, def);
// }

// template<class T>
// bool HttpRequest::checkGetParamAs(const std::string& key, T& val, const T& def = T()) {
//     initQueryParam();
//     initBodyParam();
//     return checkGetAs(paramsMap_, key, val, def);
// }
// template<class T>
// T HttpRequest::getParamAs(const std::string& key, const T& def = T()) {
//     initQueryParam();
//     initBodyParam();
//     return getAs(paramsMap_, key, def);
// }
// template<class T>
// bool HttpRequest::checkGetCookieAs(const std::string& key, T& val, const T& def = T()) {
//     initCookies();
//     return checkGetAs(cookiesMap_, key, val, def);
// }
// template<class T>
// T HttpRequest::getCookieAs(const std::string& key, const T& def = T()) {
//     initCookies();
//     return getAs(cookiesMap_, key, def);
// }


} // namespace http

} // namespace burger


#endif // HTTPREQUEST_H