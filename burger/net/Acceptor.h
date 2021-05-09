// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

// Taken from Muduo and modified

#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>
#include "burger/base/Timestamp.h"
#include <sys/stat.h>
#include <fcntl.h>
namespace burger {
namespace net {

class InetAddress;
class EventLoop;
class Channel;
class Socket;

// 主要功能create socket, bind, listen 
// 不直接使用，而是作为TcpServer成员
class Acceptor : boost::noncopyable {
public:
    // TODO: 这种传递int句柄的方法不够理想，再c++11中可以先创建Socket对象，再用移动语义把Socket对象move给callback。确保资源的安全释放
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport = true);
    ~Acceptor();
    void setNewConnectionCallback(const NewConnectionCallback& cb) { newConnectionCallback_ = cb; }
    void listen();
    bool isListening() const { return listening_; }
private: 
    void handleRead();

    EventLoop* loop_;
    std::unique_ptr<Socket> acceptSocket_;
    std::unique_ptr<Channel> acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
    int idleFd_;   // 占位fd,用于fd满的情况，避免一直电平触发
};
} // namespace net

    
} // namespace burger



#endif // ACCEPTOR_H