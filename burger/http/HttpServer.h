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
    void onRequest(const TcpConnectionPtr& conn, const HttpRequest::ptr req);
private:
    Handler* handler_;
    TcpServer server_;
    HttpCallback httpCallback_;
    std::unique_ptr<HttpRequestParser> parser_;
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
    WARN("HttpServer[ {} ] starts listening on {}", 
            server_.getHostName(), server_.getHostIpPort());
    server_.start();
}

template<typename Handler>
void HttpServer<Handler>::onConnection(const TcpConnectionPtr& conn) {
    if(conn->isConnected()) {
        std::cout << "onConnection(): new connection [" 
            << conn->getName() <<  "] from " 
            << conn->getPeerAddress().getIpPortStr() << std::endl;
    } else {
        std::cout << "onConnection() : connection " 
            << conn->getName() << " is down" << std::endl;
    }   
}

template<typename Handler>
void HttpServer<Handler>::onMessage(const TcpConnectionPtr& conn,
                    Buffer& buf,
                    Timestamp receiveTime) {
	std::string msg = buf.retrieveAllAsString();
    std::cout << msg << std::endl;
	if(!parser_->execute(&msg[0], msg.size())) {
        std::cout << "11" << std::endl;
		conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
		conn->shutdown();
	}
    std::cout << " 00" << std::endl;
	if (parser_->isFinished()) {
        std::cout << "22" << std::endl;
		onRequest(conn, parser_->getRequest());
	}
    std::cout << "33" << std::endl;
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