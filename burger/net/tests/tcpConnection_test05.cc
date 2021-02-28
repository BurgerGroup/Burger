#include "burger/net/TcpServer.h"
#include "burger/net/EventLoop.h"
#include "burger/net/InetAddress.h"

using namespace burger;
using namespace burger::net;

/*
日志分析

0, 1, 2 默认打开
3 epollFd
4 timerFd
5 wakeupFd
6 listenFd
7 idleFd
8 ... 连接的
*/

/**
 * send data and echo
 */

class TestServer {
public:
    TestServer(EventLoop* loop, const InetAddress& listenAddr) :
                    loop_(loop), 
                    server_(loop, listenAddr, "TestServer") {
        server_.setConnectionCallback(
            std::bind(&TestServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(
            std::bind(&TestServer::onMessage, this, 
            std::placeholders::_1,      // conn
            std::placeholders::_2,      // buf
            std::placeholders::_3));    // recieveTime
    }
    void start() {
        server_.start();
    }
private:
    void onConnection(const TcpConnectionPtr& conn) {
        if(conn->isConnected()) {
            std::cout << "onConnection(): new connection [" 
                << conn->getName() <<  "] from " 
                << conn->getPeerAddress().getIpPortStr() << std::endl;
        } else {
            std::cout << "onConnection() : connection " 
                << conn->getName() << " is down" << std::endl;
        }   
    }

    void onMessage(const TcpConnectionPtr& conn, 
                    Buffer& buf, 
                    Timestamp receiveTime) {
        std::string msg(buf.retrieveAllAsString());
        std::cout << "onMessage(): received " <<  msg.size() 
            << " bytes from connection " << conn->getName()
            << "at " << receiveTime.toFormatTime() << std::endl;
        conn->send(msg);   
    }   
private:
    EventLoop* loop_;
    TcpServer server_;
};

int main() {
    std::cout << "main() : pid = " << ::getpid() << std::endl;
    InetAddress listenAddr(8888);
    EventLoop loop;
    TestServer server(&loop, listenAddr);
    server.start();
    loop.loop();
}