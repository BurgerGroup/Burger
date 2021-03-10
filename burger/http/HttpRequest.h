#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "burger/base/copyable.h"
#include "burger/base/Timestamp.h"
#include "burger/base/Util.h"
#include "HttpMethod.h"
#include <map>
#include <memory>

// todo : websocket 
namespace burger {
namespace http {
class HttpRequest : public burger::copyable {
public:
    using ptr = std::shared_ptr<HttpRequest>;
    using MapType = std::map<std::string, std::string, util::CaseInsensitiveLess>;

    HttpRequest(Version version = kHttp11, bool close = true);

    enum Version {
        kUnknown, kHttp10, kHttp11
    };
    // std::string versionTostr();
    // std::shared_ptr<HttpResponse> createResponse();
    
    HttpMethod getMethod() const { return method_;}
    Version getVersion() const { return version_; }
    const std::string& getPath() { return path_; }
    const std::string& getQuery() const { return query_; }
    const std::string& getBody() const { return body_; }
    const MapType& getHeaders() const { return headers_; }
    const MapType& getParams() const { return params_; }
    const MapType& getCookies() const { return cookies_; }
    // 如果关键字key存在则返回对应值,否则返回默认值def
    std::string getHeader(const std::string& key, const std::string& def = "") const;
    std::string getParam(const std::string& key, const std::string& def = "");
    std::string getCookie(const std::string& key, const std::string& def = "");

    void setMethod(HttpMethod method) { method_ = method;}
    void setVersion(Version version) { version_ = version;}
    void setPath(const std::string& path) { path_ = path; }
    void setQuery(const std::string& query) { query_ = query; }
    void setFragment(const std::string& fragment) { fragment_ = fragment;}
    void setBody(const std::string& body) { body_ = body;}
    void setClose(bool close) { close_ = close; }
    void setHeaders(const MapType& headers) { headers_ = headers;}
    void setParams(const MapType& params) { params_ = params;}
    void setCookies(const MapType& cookies) { cookies_ = cookies;}

    void setHeader(const std::string& key, const std::string& val);
    void setParam(const std::string& key, const std::string& val);
    void setCookie(const std::string& key, const std::string& val);
    
    void delHeader(const std::string& key);
    void delParam(const std::string& key);
    void delCookie(const std::string& key);
    
    // val 如果存在,val非空则赋值
    bool hasHeader(const std::string& key, std::string* val = nullptr);
    bool hasParam(const std::string& key, std::string* val = nullptr);
    bool hasCookie(const std::string& key, std::string* val = nullptr);
    bool isClose() const { return close_; }

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
    template<class T>
    bool checkGetParamAs(const std::string& key, T& val, const T& def = T()) {
        initQueryParam();
        initBodyParam();
        return checkGetAs(m_params, key, val, def);
    }
    template<class T>
    T getParamAs(const std::string& key, const T& def = T()) {
        initQueryParam();
        initBodyParam();
        return getAs(m_params, key, def);
    }
    template<class T>
    bool checkGetCookieAs(const std::string& key, T& val, const T& def = T()) {
        initCookies();
        return checkGetAs(m_cookies, key, val, def);
    }
    template<class T>
    T getCookieAs(const std::string& key, const T& def = T()) {
        initCookies();
        return getAs(m_cookies, key, def);
    }
    // 序列化输出到流中
    std::ostream& dump(std::ostream& os) const;
    std::string toString() const;
    void init();
    void initParam();
    void initQueryParam();
    void initBodyParam();
    void initCookies();
private:    
    HttpMethod method_;   // 请求方法
    Version version_;  // http 协议版本
    bool close_;   // 是否自动关闭
    uint8_t m_parserParamFlag;  // todo
    std::string path_;   // 请求路径
    std::string query_;  // 请求参数
    std::string fragment_;  // todo 请求fragment  
    std::string body_;  // 请求消息体
    Timestamp receiveTime_;  // 请求时间
    MapType headers_;    // 请求头部列表MAP
    MapType params_;   // 请求参数MAP
    MapType cookies_;  // 请求Cookie MAP
}
// 流式输出HttpRequest
std::ostream& operator<<(std::ostream& os, const HttpRequest& req);

} // namespace http

} // namespace burger


#endif // HTTPREQUEST_H