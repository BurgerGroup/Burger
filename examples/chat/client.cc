#include "burger/net/TcpClient.h"
#include "burger/base/Log.h"
#include "burger/net/EventLoopThread.h"
#include "burger/net/IBuffer.h"
#include <boost/noncopyable.hpp>
#include <mutex>
#include <iostream>
#include <string>
// 客户端codec未拆分出来，只是当做个server的测试

using namespace burger;
using namespace burger::net;
using namespace std::placeholders;

class ChatClient : boost::noncopyable {
public:
    ChatClient(EventLoop* loop, const InetAddress& serverAddr)
        : client_(loop, serverAddr, "ChatClient") {
        client_.setConnectionCallback(
            std::bind(&ChatClient::onConnection, this, _1));
        client_.setMessageCallback(
            std::bind(&ChatClient::onMessage, this, _1, _2, _3));
        client_.enableRetry();
    }

    void connect() {
        client_.connect();
    }

    void disconnect() {
        client_.disconnect();
    }

    void write(const std::string& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (connection_) {
            wrapAndSend(connection_, msg);
        }
    }

private:
    void onConnection(const TcpConnectionPtr& conn) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (conn->isConnected()) {
            connection_ = conn;
        } else {
            connection_.reset();
        }
    }


    void onMessage(const TcpConnectionPtr& conn,
                    IBuffer& buf,
                    Timestamp receiveTime) {
        while (buf.getReadableBytes() >= kHeaderLen) {// kHeaderLen == 4
            // FIXME: use Buffer::peekInt32()
            const void* data = buf.peek();
            int32_t be32 = *static_cast<const int32_t*>(data); // SIGBUS
            const int32_t len = sockets::networkToHost32(be32);
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
                        Timestamp) {
        std::cout << "<<< " << msg << std::endl; 
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
private:    
    TcpClient client_;
    std::mutex mutex_;
    TcpConnectionPtr connection_;
    const static size_t kHeaderLen = sizeof(int32_t);
};

int main(int argc, char* argv[]) {
    if (argc > 2) {
        EventLoopThread loopThread;
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress serverAddr(argv[1], port);

        ChatClient client(loopThread.startLoop(), serverAddr);
        client.connect();
        std::string line;
        while (std::getline(std::cin, line)) {
            client.write(line);
        }
        client.disconnect();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    } else {
        std::cout << "Usage:" << argv[0] << " host_ip port" << std::endl;
    }
}

