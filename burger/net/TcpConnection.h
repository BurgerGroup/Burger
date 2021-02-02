#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <boost/noncopyable.hpp>
#include <memory>
#include <string>
#include "burger/base/Timestamp.h"
#include "InetAddress.h"
#include "Callbacks.h"

namespace burger {
namespace net {
class EventLoop;
class Socket;
class Channel;
class InetAddress;

class TcpConnection : boost::noncopyable,
            public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop* loop, 
                const std::string& connName,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
    ~TcpConnection();
    EventLoop* getLoop() const { return loop_; }
    const std::string& getName() const { return connName_; }
    const InetAddress& getLocalAddress() const { return localAddr_; }
    const InetAddress& getPeerAddress() const { return peerAddr_; }
    bool isConnected() const { return status_ == Status::kConnected; }
    
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    // internal use only 
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }
    // called when TcpServer accepts a new connection, should be called only once
    void connectEstablished();
    // called when TcpServer has removed me from its map, should be called only once
    void connectDestroyed();
private:
    enum class Status { kDisconnected, kConnecting, kConnected, kDisconnecting };
    void setStatus(Status status) { status_ = status; }
    const std::string statusToStr() const;
    
    void handleRead(Timestamp receiveTime);
    void handleClose();
    void handleError();
    EventLoop* loop_;
    const std::string connName_;
    Status status_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_; 
    const InetAddress localAddr_;
    const InetAddress peerAddr_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
} // namespace net

} // namespace burger


#endif // TCPCONNECTION_H