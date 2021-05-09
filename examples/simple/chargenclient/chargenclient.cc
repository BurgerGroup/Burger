#include "burger/base/Log.h"
#include "burger/net/EventLoop.h"
#include "burger/net/InetAddress.h"
#include "burger/net/TcpClient.h"

#include <utility>
#include <stdio.h>
#include <unistd.h>


using namespace burger;
using namespace burger::net;
using namespace std::placeholders;

class ChargenClient {
public:
    ChargenClient(EventLoop* loop, const InetAddress& listenAddr) 
        : loop_(loop),
        client_(loop, listenAddr, "ChargenClient") {
        client_.setConnectionCallback(std::bind(&ChargenClient::onConnection, this, _1));
        client_.setMessageCallback(std::bind(&ChargenClient::onMessage, this, _1, _2, _3));
        // client_.enableRetry();
    }

    void connect() {
        client_.connect();
    }

private:
    void onConnection(const TcpConnectionPtr& conn) {
        INFO("{} -> {} is {} ",  
            conn->getLocalAddress().getPortStr(), 
            conn->getPeerAddress().getPortStr(),
            (conn->isConnected() ? "UP" : "DOWN"));

        if(!conn->isConnected()) loop_->quit();
    }

    void onMessage(const TcpConnectionPtr& conn, IBuffer& buf, Timestamp receiveTime) {
        buf.retrieveAll();    // 这里就不像nc那样把数字打印到终端里
    }
private:
    EventLoop* loop_;
    TcpClient client_;
};

int main() {
    LOGGER(); LOG_LEVEL_INFO;
    EventLoop loop;
    InetAddress serverAddr("127.0.0.1", 8888);

    ChargenClient chargenClient(&loop, serverAddr);
    chargenClient.connect();
    loop.loop();
}