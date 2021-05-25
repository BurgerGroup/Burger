#include "burger/base/Log.h"
#include "burger/net/CoTcpServer.h"
#include "burger/net/Scheduler.h"
#include "burger/net/Buffer.h"
#include "codec.h"
#include <set>
#include <string>
#include <functional>
#include <mutex>
#include <boost/noncopyable.hpp>

using namespace burger;
using namespace burger::net;
using namespace std::placeholders;

class ChatServer : boost::noncopyable {
public:
    ChatServer(Scheduler* sched, const InetAddress& listenAddr)
        : server_(sched, listenAddr, "ChatServer"),
        codec_(std::bind(&ChatServer::onStringMsg, this, _1)) {
        server_.setConnectionHandler(std::bind(&ChatServer::connHandler, this, _1));
    }

    void setThreadNum(size_t threadNum) {
        server_.setThreadNum(threadNum);
    }
    void start() {
        server_.start();
    }

    void connHandler(const CoTcpConnection::ptr& conn) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            connSet_.insert(conn);
        }
        
        Buffer::ptr buffer = std::make_shared<Buffer>();
        while(conn->recv(buffer) > 0) {
            codec_.decode(conn, buffer);
        }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            connSet_.erase(conn);
        }
        
    } 

    void onStringMsg(const std::string& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        for(auto it = connSet_.begin(); it != connSet_.end(); ++it) {
            codec_.wrapAndSend(*it, msg);
        }
    }
private: 
    using ConnectionSet = std::set<CoTcpConnection::ptr>;
    CoTcpServer server_;
    LengthHeaderCodec codec_;
    ConnectionSet connSet_;  
    std::mutex mutex_;
};

int main(int argc, char* argv[]) {
    if (argc > 1) {
        Scheduler sched;
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        InetAddress serverAddr(port);
        ChatServer server(&sched, serverAddr);
        if(argc > 2) 
            server.setThreadNum(atoi(argv[2]));
        server.start();
        sched.wait();
    } else {
        printf("Usage: %s port\n", argv[0]);
    }
}

