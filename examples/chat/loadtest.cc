#include "burger/net/TcpClient.h"
#include "burger/base/Log.h"
#include "burger/net/EventLoopThread.h"
#include "burger/net/EventLoopThreadPool.h"
#include "burger/net/EventLoop.h"
#include "burger/net/IBuffer.h"
#include "burger/base/Atomic.h"
#include "burger/base/Timestamp.h"

#include <boost/noncopyable.hpp>
#include <mutex>
#include <iostream>
#include <string>
#include <memory>
#include <vector>
// 客户端codec未拆分出来，只是当做个server的测试

using namespace burger;
using namespace burger::net;
using namespace std::placeholders;

int g_connections = 0;
AtomicInt32 g_aliveConnections;
AtomicInt32 g_messagesReceived;
Timestamp g_startTime;
std::vector<Timestamp> g_receiveTime;
EventLoop* g_loop;
std::function<void()> g_statistic;

class ChatClient : boost::noncopyable {
public:
    ChatClient(EventLoop* loop, const InetAddress& serverAddr)
        : loop_(loop), 
        client_(loop, serverAddr, "LoadTestClient") {
        client_.setConnectionCallback(
            std::bind(&ChatClient::onConnection, this, _1));
        client_.setMessageCallback(
            std::bind(&ChatClient::onMessage, this, _1, _2, _3));
        // client_.enableRetry();
    }

    void connect() {
        client_.connect();
    }

    void disconnect() {
        client_.disconnect();
    }

    void stop() {
        client_.stop();
    }

    Timestamp receiveTime() const { return receiveTime_; }

private:
    void onConnection(const TcpConnectionPtr& conn) {
        if (conn->isConnected()) {
            connection_ = conn;
            if (g_aliveConnections.incrementAndGet() == g_connections) {
                INFO("all connected");
                loop_->runAfter(10.0, std::bind(&ChatClient::send, this));
            }
        } else {
            connection_.reset();
        }
    }


    void onMessage(const TcpConnectionPtr& conn,
                    IBuffer& buf,
                    Timestamp receiveTime) {
        while (buf.getReadableBytes() >= kHeaderLen) {// kHeaderLen == 4
            // FIXME: use Buffer::peekInt32()
            // const void* data = buf.peek();
            // int32_t be32 = *static_cast<const int32_t*>(data); // SIGBUS
            const int32_t len = buf.peekInt32();
            // const int32_t len = sockets::networkToHost32(be32);
            // std::cout << len << std::endl;
            if (len > 65536 || len < 0) {
                ERROR("Invalid length {}", len);
                conn->shutdown();  // FIXME: disable reading
                break;
            } else if (buf.getReadableBytes() >= len + kHeaderLen) {
                buf.retrieve(kHeaderLen);
                std::string message(buf.peek(), len);
                onStringMessage(conn, message, receiveTime);
                buf.retrieve(len);
            } else {
                break;
            }
        }
    }
    void onStringMessage(const TcpConnectionPtr&,
                        const std::string& msg,
                        Timestamp receiveTime) {
        // printf("<<< %s\n", message.c_str());
        // receiveTime_ = loop_->epollWaitRetrunTime();
        receiveTime_.swap(receiveTime);
        // assert(receiveTime_.microSecondsSinceEpoch()>0);
        int received = g_messagesReceived.incrementAndGet();
        if (received == g_connections) {
            Timestamp endTime = Timestamp::now();
            INFO("all received {} in {}",g_connections, timeDifference(endTime, g_startTime));
            g_loop->queueInLoop(g_statistic);
        } else if (received % 1000 == 0) {
            DEBUG("{}", received);
        }
    }

    void wrapAndSend(const TcpConnectionPtr& conn,
                const std::string& msg) {
        Buffer buf;
        buf.append(msg.data(), msg.size());
        int32_t len = static_cast<int32_t>(msg.size());
        int32_t be32 = sockets::hostToNetwork32(len);
        buf.prepend(&be32, sizeof be32);
        conn->send(buf);
    }

    void send() {
        g_startTime = Timestamp::now();
        wrapAndSend(connection_, "hello");
        DEBUG("sent");
    }
private:    
    EventLoop* loop_;
    TcpClient client_;
    TcpConnectionPtr connection_;
    Timestamp receiveTime_;
    const static size_t kHeaderLen = sizeof(int32_t);
};

void statistic(std::vector<std::unique_ptr<ChatClient>>& clients) {
    INFO("statistic ");
    std::vector<double> seconds(clients.size());
    for (size_t i = 0; i < clients.size(); ++i) {
        // assert(clients[i]->receiveTime().valid());
        seconds[i] = timeDifference(clients[i]->receiveTime(), g_startTime);
        // if(seconds[i] > 1) {
        //     std::cout << i << " clients ( " << clients.size() << " ) get : " << g_messagesReceived.get() << "msgs" << std::endl;
        //     printf("Abnormal value!!! ReceiveTime is %s(%lu)\n", clients[i]->receiveTime().toFormatTime().c_str(), clients[i]->receiveTime().microSecondsSinceEpoch());
        //     printf("startTime is %s\n", g_startTime.toFormatTime().c_str());
        // }
    }

    std::sort(seconds.begin(), seconds.end());
    for (size_t i = 0; i < clients.size(); i += std::max(static_cast<size_t>(1), clients.size()/20)) {
        printf("%6zd%% %.6f\n", i*100/clients.size(), seconds[i]);
    }
    if (clients.size() >= 100) {
        printf("%6d%% %.6f\n", 99, seconds[clients.size() - clients.size()/100]);
    }
    printf("%6d%% %.6f\n", 100, seconds.back());
    // why : ~Channel(): Assertion `!addedToEpoll_' failed.
    for(auto& client : clients) {
        client->disconnect();
    }
    g_loop->runAfter(5.0, std::bind(&EventLoop::quit, g_loop));
}

int main(int argc, char* argv[]) {
    // LOGGER(); LOG_LEVEL_TRACE;
    LOG_LEVEL_ERROR;
    if (argc > 3) {
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress serverAddr(argv[1], port);
        g_connections = atoi(argv[3]);
        int threads = 0;
        if (argc > 4) {
            threads = atoi(argv[4]);
        }

        EventLoop loop;
        g_loop = &loop;
        EventLoopThreadPool loopPool(&loop);
        loopPool.setThreadNum(threads);
        loopPool.start();

        g_receiveTime.reserve(g_connections);
        std::vector<std::unique_ptr<ChatClient>> clients(g_connections);
        g_statistic = std::bind(statistic, std::ref(clients));

        for (int i = 0; i < g_connections; ++i) {
            clients[i].reset(new ChatClient(loopPool.getNextLoop(), serverAddr));
            clients[i]->connect();
            usleep(200);
        }
        loop.loop();
    } else {
        printf("Usage: %s host_ip port connections [threads]\n", argv[0]);
    }
}


