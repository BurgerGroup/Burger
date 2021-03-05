
#include <functional>
#include <string>
#include <unistd.h>
#include <memory>
#include "burger/net/Channel.h"
#include "burger/net/TcpClient.h"
#include "burger/net/InetAddress.h"
#include "burger/base/Util.h"
#include "burger/net/EventLoop.h"

using namespace burger;
using namespace burger::net;

/*
从键盘接收输入  -- 前台线程

从网络接受输入 -- IO线程

todo : tcpserver崩溃后， tcpclient没重连

*/

class TestClient {
public:
    TestClient(EventLoop* loop, const InetAddress& listenAddr)
      : loop_(loop),
        client_(util::make_unique<TcpClient>(loop, listenAddr, "TestClient")),
        stdinChannel_(util::make_unique<Channel>(loop, STDIN_FILENO)) {
        client_->setConnectionCallback(std::bind(&TestClient::onConnection, 
                                        this, std::placeholders::_1));  // sockfd
        client_->setMessageCallback(std::bind(&TestClient::onMessage, this, 
                              std::placeholders::_1, 
                              std::placeholders::_2, 
                              std::placeholders::_3));
        //client_.enableRetry();
        // 标准输入缓冲区中有数据的时候，回调TestClient::handleRead
        stdinChannel_->setReadCallback(std::bind(&TestClient::handleRead, this));
        stdinChannel_->enableReading();
    }

    void connect() {
        client_->connect();
    }

private:
    void onConnection(const TcpConnectionPtr& conn) {
        if (conn->isConnected()) {
          std::cout << "onConnection(): new connection [" << conn->getName() 
              << "] from " << conn->getPeerAddress().getIpPortStr() << std::endl;
        } else {
          std::cout << "onConnection(): connection [" << conn->getName() << "] is down" << std::endl;
        }
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer& buf, Timestamp time) {
        std::string msg(buf.retrieveAllAsString());
        std::cout << "onMessage recv a message [" << msg << "]" << std::endl;
        TRACE("{} recv {} bytes at {}", conn->getName(), msg.size(), time.toFormatTime());
    }

    // 标准输入缓冲区中有数据的时候，回调该函数
    void handleRead() {
        std::string line;
        getline(std::cin, line);
        client_->getConnection()->send(line);
    }
private:
    EventLoop* loop_;
    std::unique_ptr<TcpClient> client_;
    std::unique_ptr<Channel> stdinChannel_;
};

int main(int argc, char* argv[]) {
    INFO("pid = {}, tid = {}", ::getpid(), util::tid());
    EventLoop loop;
    InetAddress serverAddr("127.0.0.1", 8888);
    // client 的accpetor具有自动重连，client先开启也可
    TestClient client(&loop, serverAddr);
    client.connect();
    loop.loop();
}