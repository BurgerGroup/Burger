#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <memory>
#include <mutex>
#include <string>
#include <boost/noncopyable.hpp>
#include "TcpConnection.h"
#include "Connector.h"

namespace burger {
namespace net {

class TcpClient : boost::noncopyable {
public:
    TcpClient(EventLoop* loop,
                const InetAddress& serverAddr,
                const std::string& name);
    ~TcpClient();  // force out-line dtor, for std::unique_ptr members.

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr connection() const;
    EventLoop* getLoop() const { return loop_; }
    bool retry() const { return retry_; }
    void enableRetry() { retry_ = true; }
    const std::string& name() const { return name_; }
    /// Not thread safe.
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    /// Not thread safe.
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    /// Not thread safe.
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
private:
    /// Not thread safe, but in loop
    void newConnection(int sockfd);
    /// Not thread safe, but in loop
    void removeConnection(const TcpConnectionPtr& conn);
private:
    EventLoop* loop_;
    ConnectorPtr connector_; // avoid revealing Connector
    const std::string name_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    bool retry_;  
    bool connect_; 
    int nextConnId_;  // todo need atomic ?
    mutable std::mutex mutex_;
    TcpConnectionPtr connection_;
};

} // namespace net

} // namespace burger

#endif // TCPCLIENT_H