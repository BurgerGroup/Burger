#include "burger/net/TcpServer.h"
#include "burger/net/EventLoop.h"
#include "burger/net/InetAddress.h"
#include <functional>

using namespace burger;
using namespace burger::net;

class TestServer {
public:
    TestServer(EventLoop* loop,
                const InetAddress& listenAddr)
        : loop_(loop),
        server_(loop, listenAddr, "TestServer") {
        server_.setConnectionCallback(
            std::bind(&TestServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&TestServer::onMessage, this, 
                    std::placeholders::_1,  // conn 
                    std::placeholders::_2,  // buffer
                    std::placeholders::_3)); // recieveTime
        server_.setWriteCompleteCallback(std::bind(&TestServer::onWriteComplete, this, std::placeholders::_1));

        // 不断生成数据
        std::string line;
        for (int i = 33; i < 127; ++i) {
            line.push_back(char(i));
        }
        line += line;

        for (size_t i = 0; i < 127-33; ++i) {
            message_ += line.substr(i, 72) + '\n';
        }
    }

    void start() {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn) {
        if (conn->isConnected()) {
            std::cout << "onConnection(): new connection [" 
                << conn->getName() <<  "] from " 
                << conn->getPeerAddress().getIpPortStr() << std::endl;

            conn->setTcpNoDelay(true);
            conn->send(message_);
        } else {
            std::cout << "onConnection() : connection " 
                << conn->getName() << " is down" << std::endl;
        }
    }

    void onMessage(const TcpConnectionPtr& conn,
                    Buffer& buf,
                    Timestamp recieveTime) {
        std::string msg(buf.retrieveAllAsString());
        std::cout << "onMessage(): received " << buf.getReadableBytes() 
            << " bytes from connection " << conn->getName() 
            << " at " << recieveTime.toFormatTime() <<std::endl;
        conn->send(msg);
    }

    void onWriteComplete(const TcpConnectionPtr& conn) {
        conn->send(message_);
    }

private:
    EventLoop* loop_;
    TcpServer server_;
    std::string message_;
};


int main() {
    std::cout << "main() : pid = " << ::getpid() << std::endl;
    InetAddress listenAddr(8888);
    EventLoop loop;
    TestServer server(&loop, listenAddr);
    server.start();
    loop.loop();
}