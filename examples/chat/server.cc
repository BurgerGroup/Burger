#include "burger/base/Log.h"
#include "burger/net/CoTcpServer.h"
#include "burger/net/Scheduler.h"
#include "burger/net/Buffer.h"
#include "codec.h"
#include <set>
#include <string>
#include <functional>
#include <boost/noncopyable.hpp>

using namespace burger;
using namespace burger::net;
using namespace std::placeholders;

class ChatServer : boost::noncopyable {
public:
    ChatServer(Scheduler* sched, const InetAddress& listenAddr)
        : sched_(sched),
        server_(sched, listenAddr, "ChatServer"),
        codec_(std::bind(&ChatServer::onStringMsg, this, _1)) {
        server_.setConnectionHandler(std::bind(&ChatServer::connHandler, this, _1));
    }

    void start() {
        server_.start();
    }

    void connHandler(const CoTcpConnection::ptr& conn) {
        connections_.insert(conn);
        codec_.wrapAndSend(conn, "Connected!");
        Buffer::ptr buffer = std::make_shared<Buffer>();
        while(conn->recv(buffer) > 0) {
            codec_.decode(conn, buffer);
        }
        connections_.erase(conn);
    } 

    void onStringMsg(const std::string& msg) {
        for(auto it = connections_.begin(); it != connections_.end(); ++it) {
            codec_.wrapAndSend(*it, msg);
        }
    }
private: 
    using ConnectionList = std::set<CoTcpConnection::ptr>;
    Scheduler* sched_;
    CoTcpServer server_;
    LengthHeaderCodec codec_;
    ConnectionList connections_;  
};

int main(int argc, char* argv[]) {
    if (argc > 1) {
        Scheduler sched;
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        InetAddress serverAddr(port);
        ChatServer server(&sched, serverAddr);
        server.start();
        sched.wait();
    } else {
        printf("Usage: %s port\n", argv[0]);
    }
}

