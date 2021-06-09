// shared_ptr实现copy on write

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
        codec_(std::bind(&ChatServer::onStringMsg, this, _1)),
        connSetPtr_(std::make_shared<ConnectionSet>()) {   // use_count 为 1 
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
            // 在复本上修改，不会影响读者，所以读者在遍历列表的时候，不需要用mutex保护
            std::lock_guard<std::mutex> lock(mutex_);
            if(!connSetPtr_.unique()) {  // 引用计数大于1
                connSetPtr_.reset(new ConnectionSet(*connSetPtr_)); // 此处是精髓
            }
            assert(connSetPtr_.unique());
            connSetPtr_->insert(conn);
        }
        
        Buffer::ptr buffer = std::make_shared<Buffer>();
        while(conn->recv(buffer) > 0) {
            codec_.decode(conn, buffer);
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if(!connSetPtr_.unique()) {  // 引用计数大于1
                connSetPtr_.reset(new ConnectionSet(*connSetPtr_)); // 此处是精髓
            }
            assert(connSetPtr_.unique());
            connSetPtr_->erase(conn);
        }
    } 

    void onStringMsg(const std::string& msg) {
        // 引用计数加1，mutex保护的临界区大大缩短
        // 写者是在另一个复本上修改，所以写者无需担心更改了连接的列表
        ConnectionSetPtr connSetPtr = getConnSetPtr();
        for(auto it = (*connSetPtr).begin(); it != (*connSetPtr).end(); ++it) {
            codec_.wrapAndSend(*it, msg);
        }
        // 这个断言不一定成立, 不能确定之前到达reset没有
        // assert(!connections.unique());
        // 当ConnectionSetPtr这个栈上的变量销毁的时候，引用计数减1
    }

    using ConnectionSet = std::set<CoTcpConnection::ptr>;
    using ConnectionSetPtr = std::shared_ptr<ConnectionSet>;

    ConnectionSetPtr getConnSetPtr() {
        std::lock_guard<std::mutex> lock(mutex_);
        return connSetPtr_;
    }
private: 
    CoTcpServer server_;
    LengthHeaderCodec codec_;
    ConnectionSetPtr connSetPtr_;
    std::mutex mutex_;
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

