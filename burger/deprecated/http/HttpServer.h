#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>
#include <string>
#include "burger/net/TcpServer.h"
#include "HttpRequestParser.h"
#include "burger/net/EventLoop.h"
#include "burger/net/InetAddress.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpRequestParser.h"
#include "Version.h"
#include <iostream>

using namespace std::placeholders;
namespace burger {
namespace http {

namespace detail {

void defaultHttpCallback(const HttpRequest::ptr, HttpResponse* resp) {
    resp->setStatus(HttpStatus::NOT_FOUND);
    resp->setReason(HttpStatusToString(HttpStatus::NOT_FOUND));
    resp->setClose(true);  // todo
}

} // namespace detail

using namespace burger::net;
template <typename Handler>
class HttpServer : boost::noncopyable {
public:
    using HttpCallback = std::function<void (const HttpRequest::ptr, HttpResponse*)> ;
    HttpServer(Handler* handler, EventLoop* loop,
                const InetAddress& listenAddr,
                const std::string& name);
    EventLoop* getLoop() const { return server_.getLoop(); }
    void setHttpCallback(const HttpCallback& cb) { httpCallback_ = cb; }
    void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }
    
    void start();
private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn,
                    Buffer& buf,
                    Timestamp receiveTime);
    HttpRequest::ptr receiveRequest(const TcpConnectionPtr& conn, HttpRequestParser::ptr parser, Buffer& buf);
    void sendResponse(const TcpConnectionPtr& conn, HttpRequest::ptr req);
    void onRequest(const TcpConnectionPtr& conn, const HttpRequest::ptr req);
private:
    Handler* handler_;
    TcpServer server_;
    HttpCallback httpCallback_;
    // std::unique_ptr<HttpRequestParser> parser_;  // todo 为什么不设置相同的parser_在此
};

template<typename Handler>
HttpServer<Handler>::HttpServer(Handler* handler, EventLoop* loop,
                const InetAddress& listenAddr,
                const std::string& name) 
	: handler_(handler), 
    server_(loop, listenAddr, name),
    httpCallback_(detail::defaultHttpCallback) {
    server_.setConnectionCallback(
        std::bind(&HttpServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&HttpServer::onMessage, this, _1, _2, _3));
}



template<typename Handler>
void HttpServer<Handler>::start()  {
    TRACE("HttpServer[ {} ] starts listening on {}", 
            server_.getHostName(), server_.getHostIpPort());
    server_.start();
}

template<typename Handler>
void HttpServer<Handler>::onConnection(const TcpConnectionPtr& conn) {
    if(conn->isConnected()) {
        auto parser = std::make_shared<HttpRequestParser>();
        conn->setContext(parser);
        // 封装成connectionCallback??
        TRACE("onConnection(): new connection [{}] from {}", conn->getName(), conn->getPeerAddress().getIpPortStr());
    } else {
        TRACE("onConnection() : connection {} is down", conn->getName());
        auto requestParser = conn->getContext<HttpRequestParser>();
        if (requestParser) {
            conn->clearContext();
        }
    }   
}

template<typename Handler>
void HttpServer<Handler>::onMessage(const TcpConnectionPtr& conn,
                    Buffer& buf,
                    Timestamp receiveTime) {
    if (!conn->hasContext()) return;
    auto requestParser = conn->getContext<HttpRequestParser>();
    if (!requestParser) return;
    auto req = receiveRequest(conn, requestParser, buf); 
    sendResponse(conn, req);
}

template<typename Handler>
HttpRequest::ptr HttpServer<Handler>::receiveRequest(const TcpConnectionPtr& conn, HttpRequestParser::ptr parser, Buffer& buf) {
    // uint64_t buffSize = HttpRequestParser::GetHttpRequestBufferSize();
    do {
        size_t nparse = parser->execute(buf.beginRead(), buf.getReadableBytes());
        // if(nparse > buffSize) {
        //     ERROR("Exceed the maximum buffer size, connection close");
        //     buf.retriveAll();
        //     parser->reset();
        //     conn->forceClose();
        //     return nullptr;
        // }
        buf.retrieve(nparse);
        if(parser->hasError()) {
            conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
            conn->shutdown();
            return nullptr;
        }
        if(parser->isFinished()) {
            break;
        }
    } while(true);
    uint64_t contenLength = parser->getContentLength();
    // std::cout << parser->getRequest()->toString() << std::endl;  // for test
    // std::cout << buf.retrieveAsString(static_cast<size_t>(contenLength)) << std::endl;  // for test
    parser->getRequest()->setBody(buf.retrieveAsString(static_cast<size_t>(contenLength)));
    return parser->getRequest();
}

template<typename Handler>
void HttpServer<Handler>::sendResponse(const TcpConnectionPtr& conn, HttpRequest::ptr req) {
    HttpResponse::ptr rsp = std::make_shared<HttpResponse>(req->isClose(), req->getVersion());
    rsp->setHeader("Server", server_.getHostName());
    rsp->setBody("hello");
    std::cout << rsp->toString() << std::endl;
    // conn->send(rsp->toString());
}

template<typename Handler>
void HttpServer<Handler>::onRequest(const TcpConnectionPtr& conn, const HttpRequest::ptr req) {
	const std::string& connection = req->getHeader("Connection");
	bool close = connection == "close" || 
		(req->getVersion() == Version::kHttp10 && connection != "Keep-Alive");
	HttpResponse response(close);
	httpCallback_(req, &response);
	conn->send(response.toString());
	if(response.isClose()) {
		conn->shutdown();
	}
}

    
} // namespace http
} // namespace burger




#endif // HTTPSERVER_H