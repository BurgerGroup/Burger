#include "burger/net/TcpClient.h"
#include "burger/base/Atomic.h"
#include "burger/base/Log.h"
#include "burger/net/EventLoop.h"
#include "burger/net/EventLoopThreadPool.h"
#include "burger/net/InetAddress.h"
#include "burger/net/RingBuffer.h"

#include <utility>
#include <string>
#include <stdio.h>
#include <unistd.h>

using namespace burger;
using namespace burger::net;

class Client;

class Session {   // 一个Client包含可以发起多个session，每个session由一个TcpClient建立并维护
public:
    Session(EventLoop* loop,
        const InetAddress& serverAddr,
        const std::string& name,
        Client* owner)
    : client_(loop, serverAddr, name),
      owner_(owner),
      bytesRead_(0),
      bytesWritten_(0),
      messagesRead_(0)
    {
        client_.setConnectionCallback(
            std::bind(&Session::onConnection, this, std::placeholders::_1));
        client_.setMessageCallback(
            std::bind(&Session::onMessage, this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3));
    }

    void start() {
        client_.connect();
    }

    void stop() {
        client_.disconnect();
    }

    int64_t bytesRead() const {
        return bytesRead_;
    }

    int64_t messagesRead() const {
        return messagesRead_;
    }

private:

    void onConnection(const TcpConnectionPtr& conn);

    void onMessage(const TcpConnectionPtr& conn, IBuffer& buf, Timestamp) {
        ++messagesRead_;
        bytesRead_ += buf.getReadableBytes();
        bytesWritten_ += buf.getReadableBytes();
        conn->send(buf);
    }

    TcpClient client_;
    Client* owner_;
    int64_t bytesRead_;
    int64_t bytesWritten_;
    int64_t messagesRead_;
};

class Client {
public:
    Client(EventLoop* loop,
            const InetAddress& serverAddr,
            int blockSize,
            int sessionCount,
            int timeout,
            int threadCount)
        : loop_(loop),
        threadPool_(loop),
        sessionCount_(sessionCount),
        timeout_(timeout)
    {
        loop->runAfter(timeout, std::bind(&Client::handleTimeout, this));
        if (threadCount > 1) {
            threadPool_.setThreadNum(threadCount);
        }
        threadPool_.start();

        for (int i = 0; i < blockSize; ++i) {
            message_.push_back(static_cast<char>(i % 128));
        }

        for (int i = 0; i < sessionCount; ++i) {
            char buf[32];
            snprintf(buf, sizeof(buf), "C%05d", i);
            Session* session = new Session(threadPool_.getNextLoop(), serverAddr, buf, this);
            session->start();
            sessions_.emplace_back(session);
        }
    }

    const std::string& message() const {
        return message_;
    }

    void onConnect() {
        if (numConnected_.incrementAndGet() == sessionCount_) {
            WARN("all connected");
        }
    }

    void onDisconnect(const TcpConnectionPtr& conn) {
        if (numConnected_.decrementAndGet() == 0) {
        WARN("all disconnected");

        int64_t totalBytesRead = 0;
        int64_t totalMessagesRead = 0;
        for (const auto& session : sessions_) {
            totalBytesRead += session->bytesRead();
            totalMessagesRead += session->messagesRead();
        }
        WARN("{} total bytes read", totalBytesRead);
        WARN("{} total messages read", totalMessagesRead);
        WARN("{} average message size", static_cast<double>(totalBytesRead) / static_cast<double>(totalMessagesRead));
        WARN("{} MiB/s throughput", static_cast<double>(totalBytesRead) / (timeout_ * 1024 * 1024));
        conn->getLoop()->queueInLoop(std::bind(&Client::quit, this));
        }
    }

private:

    void quit() {
        loop_->queueInLoop(std::bind(&EventLoop::quit, loop_));
    }

    void handleTimeout() {
        WARN("stop");
        for (auto& session : sessions_) {
            session->stop();
        }
    }

    EventLoop* loop_;
    EventLoopThreadPool threadPool_;
    int sessionCount_;
    int timeout_;
    std::vector<std::unique_ptr<Session>> sessions_;
    std::string message_;
    AtomicInt32 numConnected_;
};

void Session::onConnection(const TcpConnectionPtr& conn) {
    if (conn->isConnected()) {
        conn->setTcpNoDelay(true);
        // printf("msg.length() = %lu\n", owner_->message().length());
        conn->send(owner_->message());
        owner_->onConnect();
    }
    else {
        owner_->onDisconnect(conn);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Usage: client <host_ip> <port> <threads> <blocksize> ");
        fprintf(stderr, "<sessions> <time>\n");
    }
    else {
        INFO("pid = {}, tid = {}", getpid(), util::tid());
        LOGGER("./pingpong_client.log", "pingpong client");
        LOG_LEVEL_WARN;

        const char* ip = argv[1];
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        int threadCount = atoi(argv[3]);
        int blockSize = atoi(argv[4]);
        int sessionCount = atoi(argv[5]);
        int timeout = atoi(argv[6]);

        EventLoop loop;
        InetAddress serverAddr(ip, port);

        Client client(&loop, serverAddr, blockSize, sessionCount, timeout, threadCount);
        loop.loop();
    }
}

