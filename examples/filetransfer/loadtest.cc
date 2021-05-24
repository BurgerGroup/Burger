#include "burger/base/Atomic.h"
#include "burger/base/Log.h"
#include "burger/net/EventLoop.h"
#include "burger/net/EventLoopThreadPool.h"
#include "burger/net/TcpClient.h"
#include "burger/base/Util.h"
#include <boost/noncopyable.hpp>

#include <vector>
#include <string>
#include <functional>
#include <assert.h>
#include <stdio.h>

using namespace burger;
using namespace burger::net;
using namespace std::placeholders;

AtomicInt32 g_aliveConnections;
AtomicInt32 g_disaliveConnections;
int g_connections = 4;
EventLoop* g_loop;

class RecvFileClient : boost::noncopyable {
public:
    RecvFileClient(EventLoop* loop, const InetAddress& serverAddr, const std::string& id)
        : loop_(loop),
        client_(loop, serverAddr, "RecvFileClient") {
        client_.setConnectionCallback(
            std::bind(&RecvFileClient::onConnection, this, _1));
        client_.setMessageCallback(
            std::bind(&RecvFileClient::onMessage, this, _1, _2, _3));
        std::string filename = "RecvFileClient" + id;
        fp_ = ::fopen(filename.c_str(), "we");
        assert(fp_);
    }

    ~RecvFileClient() {
        ::fclose(fp_);
    }

    void connect() {
        client_.connect();
    }

private:
    void onConnection(const TcpConnectionPtr& conn) {
        if (conn->isConnected()) {
            connection_ = conn;
            if (g_aliveConnections.incrementAndGet() == g_connections)
                INFO("all connected");
        } else {
            connection_.reset();
            if (g_disaliveConnections.incrementAndGet() == g_connections) {
                INFO("all disconnected");
                g_loop->quit();
                //exit(0);  // 第二种退出方法
            }
        }
    }

    void onMessage(const TcpConnectionPtr& conn, IBuffer& buf, Timestamp time) {
        fwrite(buf.peek(), 1, buf.getReadableBytes(), fp_);
        buf.retrieveAll();
    }

    EventLoop* loop_;
    TcpClient client_;
    TcpConnectionPtr connection_;
    FILE* fp_;
};

int main(int argc, char* argv[]) {
    LOGGER(); LOG_LEVEL_TRACE;
    EventLoop loop;
    g_loop = &loop;
    // 用两个IO线程来发起大量的连接
    EventLoopThreadPool loopPool(&loop);
    loopPool.setThreadNum(2);
    loopPool.start();
    
    std::vector<std::unique_ptr<RecvFileClient> > clients(g_connections);

    InetAddress serverAddr("127.0.0.1", 8888);

    for (int i = 0; i < g_connections; ++i) {
        std::string id = std::to_string(i+1);
        clients[i] = util::make_unique<RecvFileClient>(loopPool.getNextLoop(), serverAddr, id);
        clients[i]->connect();
        usleep(200);
    }


    loop.loop();
    usleep(20000);
}