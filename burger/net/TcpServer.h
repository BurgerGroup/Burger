#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <boost/noncopyable.hpp>
#include <map>
#include <string>
#include "Callbacks.h"
#include "TcpConnection.h"
#include "burger/base/Atomic.h"

namespace burger {
namespace net {
class EventLoop;
class InetAddress;
class Acceptor;
class EventLoopThreadPool;

class TcpServer : boost::noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    TcpServer(EventLoop* loop, const InetAddress& listenAddr, 
                            const std::string& name, bool reuseport = true);
    ~TcpServer();

    const std::string& getHostIpPort() const { return hostIpPort_; }
    const std::string& getHostName() const { return hostName_; }

    void setThreadNum(int numThreads);
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }
    // not thread safe
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    // not thread safe 
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

    void start();
private:
    // Not thread safe, but in loop，连接时回调的函数
    void newConnection(int sockfd, const InetAddress& peerAddr);
    // thread safe 
    void removeConnection(const TcpConnectionPtr& conn);
    // not threadsafe, but in loop
    void removeConnectionInLoop(const TcpConnectionPtr& conn);
    
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    EventLoop* loop_;  // the acceptor loop, 不一定是连接所属的
    const std::string hostIpPort_;
    const std::string hostName_;
    std::unique_ptr<Acceptor> acceptor_;
    std::unique_ptr<EventLoopThreadPool> threadPool_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    ThreadInitCallback threadInitCallback_;
    AtomicInt32 started_;  // in start() avoid race condition 
    int nextConnId_;
    ConnectionMap connectionsMap_;
};
} // namespace net
} // namespace burger




#endif // TCPSERVER_H