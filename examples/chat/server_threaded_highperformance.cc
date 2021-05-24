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

class ChatServer : boost::noncopyable {
public:
    ChatServer(Scheduler* sched, const InetAddress& listenAddr)
        : server_(sched, listenAddr, "ChatServer"),
        codec_(std::bind(&ChatServer::onStringMsg, this, _1)) {   // use_count 为 1 
        server_.setConnectionHandler(std::bind(&ChatServer::connHandler, this, _1));
    }

    void setThreadNum(size_t threadNum) {
        server_.setThreadNum(threadNum);
    }

    void start() {
        server_.start();
        workProcList_ = server_.getScheduler()->getWorkProcList();
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
        std::lock_guard<std::mutex> lock(mutex_);
        for(auto it = workProcList_.begin(); it != workProcList_.end(); it++) {
            (*it)->addTask(std::bind(&ChatServer::distributeMsg, this, msg), "distribute msg");
        }
        
    }

    void distributeMsg(const std::string& msg) {
        DEBUG("BEGIN");
        for(auto it = connSetPerThrd::Instance().begin(); 
                it != connSetPerThrd::Instance().end(); it++) {
            codec_.wrapAndsend(*it, msg);        
        }
        DEBUG("END");
    }

private: 
    using ConnectionSet = std::set<CoTcpConnection::ptr>;
    using connSetPerThrd = SingletonPerThread<ConnectionSet>;
    CoTcpServer server_;
    LengthHeaderCodec codec_;
    std::mutex mutex_;
    std::vector<Processor *> workProcList_;
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

