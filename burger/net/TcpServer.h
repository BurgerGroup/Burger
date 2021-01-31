#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <boost/noncopyable.hpp>
#include <map>
#include <string>
#include "Callbacks.h"
#include "TcpConnection.h"

namespace burger {
namespace net {
class EventLoop;
class InetAddress;
class Acceptor;

class TcpServer : boost::noncopyable {
public:
    TcpServer();
    ~TcpServer();

    // not thread safe
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    // not thread safe 
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

    void start();
private:
    // Not thread safe, but in loop，连接时回调的函数
    void newConnection(int sockfd, const InetAddress& peerAddr);
    using ConnectionMap = std::map<std::string, TcpConnection>;

    EventLoop* loop_;  // the acceptor loop, 不一定是连接所属的
    const std::string hostport_;
    const std::string hostname;
    std::unique_ptr<Acceptor> acceptor_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    bool started_;  // TODO : why use AtomicInt32?
    int nextConnId_;
    ConnectionMap connectionsMap_;
};
} // namespace net

} // namespace burger




#endif // TCPSERVER_H