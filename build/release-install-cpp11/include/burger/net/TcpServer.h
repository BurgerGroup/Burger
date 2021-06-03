// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

// Taken from Muduo and modified

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

// TcpServer 管理accept获得的TcpConnection
// 供用户使用，生命期由用户控制，用户只需设置好callback,再调用start()即可
class TcpServer : boost::noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    TcpServer(EventLoop* loop, const InetAddress& listenAddr, 
                            const std::string& name, bool reuseport = true);
    ~TcpServer();

    const std::string& getHostIpPort() const { return hostIpPort_; }
    const std::string& getHostName() const { return hostName_; }
    EventLoop* getLoop() const { return loop_; }
    
    void setThreadNum(int numThreads);
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }
    // not thread safe
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    // not thread safe 
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
    void start();
private:
    // Not thread safe, but in loop，连接时回调的函数
    void newConnection(int sockfd, const InetAddress& peerAddr);
    // thread safe 
    void removeConnection(const TcpConnectionPtr& conn);
    // not threadsafe, but in loop
    void removeConnectionInLoop(const TcpConnectionPtr& conn);
    
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;  // 每个TcpConnection都有一个名字，作为key

    EventLoop* loop_;  // the acceptor loop, 不一定是连接所属的
    const std::string hostIpPort_;
    const std::string hostName_;
    std::unique_ptr<Acceptor> acceptor_;  // 用于获取新连接
    std::unique_ptr<EventLoopThreadPool> threadPool_;
    // 保存用户提供的ConnectionCallback和 MessageCallback
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;
    AtomicInt32 started_;  // in start() avoid race condition 
    int nextConnId_;
    ConnectionMap connectionsMap_;  // 持有目前存活的TcpConnection ptr(生命期模糊，TcpServer可持有，用户也可持有)
};
} // namespace net
} // namespace burger




#endif // TCPSERVER_H