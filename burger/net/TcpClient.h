// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

// Taken from Muduo and modified

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

    TcpConnectionPtr getConnection() const;
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
    ConnectorPtr connector_; // 用于主动发起连接
    const std::string name_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    bool retry_;  // 与connector(还没连接成功是否重连)不一样，这里是建立连接成功后，又断开的时候是否重连，
    bool connect_; 
    int nextConnId_;  // always in loop thread，所以不需要atomic, name_ + connId 标识一个连接
    mutable std::mutex mutex_;
    TcpConnectionPtr connection_; // connector连接成功后建立一个connection
};

} // namespace net

} // namespace burger

#endif // TCPCLIENT_H