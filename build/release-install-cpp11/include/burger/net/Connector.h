// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

// Taken from Muduo and modified

#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <memory>
#include <functional>
#include <boost/noncopyable.hpp>
#include "burger/net/InetAddress.h"

namespace burger {
namespace net {
class EventLoop;
class InetAddress;
class Channel;
class Connector : boost::noncopyable,
                public std::enable_shared_from_this<Connector> {
public:
    using NewConnectionCallback = std::function<void (int sockfd)>;

    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_ = cb; }

    void start();  // can be called in any thread
    void restart();  // must be called in loop thread
    void stop();  // can be called in any thread

    const InetAddress& getServerAddress() const { return serverAddr_; }

private:
    enum class Status { kDisconnected, kConnecting, kConnected };
    void setStatus(Status status) { status_ = status; }
    const std::string statusToStr() const;

    // https://stackoverflow.com/questions/16957458/static-const-in-c-class-undefined-reference/16957554
    static const int kMaxRetryDelayMs = 30*1000;  // 30s, 最大重连延迟时间
    static const int kInitRetryDelayMs = 500;  // 0.5s, 初始状态，连接不上，05s重连
    
    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();
private:
    EventLoop* loop_;
    InetAddress serverAddr_;
    bool connect_; // atomic
    Status status_;  // FIXME: use atomic variable
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback_;  // 连接成功回调
    int retryDelayMs_;   // 重连延迟事件 ms
};

using ConnectorPtr = std::shared_ptr<Connector>;

} // namespace net

} // namespace burger



#endif // CONNECTOR_H