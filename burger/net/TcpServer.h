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

class TcpServer : boost::noncopyable {
public:
    TcpServer(EventLoop* loop, const InetAddress& listenAddr, 
                                std::string name, bool reuseport = true);
    ~TcpServer();

    const std::string& getHostIpPort() const { return hostIpPort_; }
    const std::string& getHostName() const { return hostName_; }

    // not thread safe
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    // not thread safe 
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

    void start();
private:
    // Not thread safe, but in loop，连接时回调的函数
    void newConnection(int sockfd, const InetAddress& peerAddr);
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    EventLoop* loop_;  // the acceptor loop, 不一定是连接所属的
    const std::string hostIpPort_;
    const std::string hostName_;
    std::unique_ptr<Acceptor> acceptor_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    AtomicInt32 started_;  // in start() avoid race condition 
    int nextConnId_;
    ConnectionMap connectionsMap_;
};
} // namespace net

} // namespace burger




#endif // TCPSERVER_H