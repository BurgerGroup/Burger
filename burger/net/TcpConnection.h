#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <boost/noncopyable.hpp>
#include <memory>
#include <string>
#include "burger/base/Timestamp.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

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
    bool getTcpInfo(struct tcp_info&) const;
    std::string getTcpInfoString() const;
    void send(const std::string& message);
    // void send(std::string&& message);  // todo
    void send(Buffer& buf);
    // void send(Buffer&& message);  // todo
    void shutdown(); // NOT thread safe
    void forceClose();
    void forceCloseWithDelay(double seconds);
    void setTcpNoDelay(bool on);
    
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
    void sendInLoop(const std::string& message);
    void sendInLoop(const void* data, size_t len);
    EventLoop* loop_;
    const std::string connName_;
    Status status_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_; 
    const InetAddress localAddr_;
    const InetAddress peerAddr_;
    ConnectionCallback connectionCallback_;   // 连接建立和关闭时的回调函数
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;   // 消息写入对方缓冲区是的回调函数
    CloseCallback closeCallback_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
} // namespace net

} // namespace burger


#endif // TCPCONNECTION_H