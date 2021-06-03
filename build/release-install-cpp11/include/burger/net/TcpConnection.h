// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

// Taken from Muduo and modified
#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <boost/noncopyable.hpp>
#include <memory>
#include <string>
#include <typeinfo>
#include "burger/base/Timestamp.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "IBuffer.h"
#include "Buffer.h"
#include "RingBuffer.h"
#include <boost/any.hpp>
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
    // void send(Buffer& buf);
    void send(IBuffer& buf);
    // void send(Buffer&& message);  // todo
    void shutdown(); // NOT thread safe
    void forceClose();
    void forceCloseWithDelay(double seconds);
    void setTcpNoDelay(bool on);
    
    void setContext(const std::shared_ptr<void> &context) { contextPtr_ = context; }
    void setContext(std::shared_ptr<void> &&context) { contextPtr_ = std::move(context); }
    template <typename T>
    std::shared_ptr<T> getContext() const {
        return std::static_pointer_cast<T>(contextPtr_);
    }
    bool hasContext() const {
        return static_cast<bool>(contextPtr_);
    }
    void clearContext() { contextPtr_.reset(); }

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark) 
        { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }
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
    void handleWrite();
    void handleClose();
    void handleError();
    void sendInLoop(const std::string& message);
    void sendInLoop(const void* data, size_t len);
    void shutdownInLoop();
    void forceCloseInLoop();
    EventLoop* loop_;
    const std::string connName_;
    Status status_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_; 
    const InetAddress localAddr_;
    const InetAddress peerAddr_;
    ConnectionCallback connectionCallback_;   // 连接建立和关闭时的回调函数
    MessageCallback messageCallback_;
    // 如果不断生成数据，发送conn->send
    // 如果对等接收不及时，受到通告窗口的的控制，内核发送缓冲不足，这个时候，就会将用户数据添加到output buffer
    // 解决办法 调整发送频率， 关注writeCompleteCallback_。
    WriteCompleteCallback writeCompleteCallback_;   // 消息写入对方缓冲区是的回调函数, output buffer 清空也会回调  -- 低水位
    HighWaterMarkCallback highWaterMarkCallback_;  // output buffer 撑到一定程度回调
    CloseCallback closeCallback_;
    size_t highWaterMark_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    // RingBuffer inputBuffer_;
    // RingBuffer outputBuffer_;
    // https://blog.csdn.net/Solstice/article/details/6384968
    // 可变类型解决方案 ： 1. void* 这种方法不是类型安全 2. boost::any  -- 任意类型的类型安全存储和安全的取回，在标准容器中存放不同类型的方法，比如vector<boost::any>
    std::shared_ptr<void> contextPtr_;
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
} // namespace net

} // namespace burger


#endif // TCPCONNECTION_H