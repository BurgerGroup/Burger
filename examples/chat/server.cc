#include "burger/base/Log.h"
#include "burger/net/CoTcpServer.h"
#include "burger/net/Scheduler.h"

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
        : server_(sched, listenAddr, "ChatServer");,
        codec_(std::bind(&ChatServer::onStringMsg, this, _1)) {
        server_.setConnectionHandler(std::bind(&ChatServer::connHandler, this, _1));
    }

    void connHandler(const CoTcpConnection::ptr& conn) {
        connections_.insert(conn);
        RingBuffer::ptr buffer = std::make_shared<RingBuffer>();
        while(conn->recv(buffer) > 0) {
            codec_.decode(conn, buffer);
        }
    } 

    void onStringMsg(const std::string& msg) {
        for(auto it = connections_.being(); it != connections_.end(); ++it) {
            codec_.send(*it, msg);
        }
    }
private: 
    using ConnectionList = std::set<CoTcpConnection::ptr>;
    CoTcpServer server_;
    LengthHeaderCodec codec_;
    ConnectionList connections_;
    
};