// thread_local 实现多线程高效转发

#include "burger/base/Log.h"
#include "burger/base/Singleton.h"
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

// todo : 此处性能表现不佳，分析原因

class ChatServer : boost::noncopyable {
public:
    ChatServer(Scheduler* sched, const InetAddress& listenAddr)
        : sched_(sched),
        server_(sched, listenAddr, "ChatServer"),
        codec_(std::bind(&ChatServer::onStringMsg, this, _1)) {   // use_count 为 1 
        server_.setConnectionHandler(std::bind(&ChatServer::connHandler, this, _1));
    }

    void setThreadNum(size_t threadNum) {
        server_.setThreadNum(threadNum);
    }

    void start() {
        server_.start();
    }

    void connHandler(const CoTcpConnection::ptr& conn) {
        connSetPerThrd::Instance().insert(conn);
        Buffer::ptr buffer = std::make_shared<Buffer>();
        while(conn->recv(buffer) > 0) {
            codec_.decode(conn, buffer);
        }
        connSetPerThrd::Instance().erase(conn);
    } 

    void onStringMsg(const std::string& msg) {
        sched_->distributeTask(std::bind(&ChatServer::distributeMsg, this, msg), "distribute msg");
    }

    void distributeMsg(const std::string& msg) {
        DEBUG("BEGIN");
        for(auto it = connSetPerThrd::Instance().begin(); 
                it != connSetPerThrd::Instance().end(); ++it) {
            codec_.wrapAndSend(*it, msg);        
        }
        // for test
        // Processor* proc = (*(connSetPerThrd::Instance().begin()))->getProc();
        // printf("Totol create times is : %lu\n", proc->getCreateTimes());
        DEBUG("END");
    }

private: 
    using ConnectionSet = std::set<CoTcpConnection::ptr>;
    using connSetPerThrd = SingletonPerThread<ConnectionSet>;
    Scheduler *sched_;
    CoTcpServer server_;
    LengthHeaderCodec codec_;
    // std::mutex mutex_;
};

int main(int argc, char* argv[]) {
    LOG_LEVEL_ERROR;
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

