// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

// Taken from Muduo and modified
#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <functional>
#include <memory>
#include "burger/base/Timestamp.h"

namespace burger {
namespace net {

// All client visible callbacks go here.

class TcpConnection;
class IBuffer;
class CoTcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using CoTcpConnectionPtr = std::shared_ptr<CoTcpConnection>;

using TimerCallback = std::function<void()> ;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void (const TcpConnectionPtr&)>;
using HighWaterMarkCallback = std::function<void (const TcpConnectionPtr&, size_t)>;
using MessageCallback = std::function<void (const TcpConnectionPtr&,
                                                    IBuffer&,
                                                    Timestamp)>;

// using CoConnEstablishCallback = std::function<void(const CoTcpConnectionPtr&)>;


// using MessageCallback = std::function<void (const TcpConnectionPtr&,
//                                                     RingBuffer&,
//                                                     Timestamp)>;
// 声明于此，实现在Tcpconnection里面, 然后在Tcpserver里设置
// TODO : 为何这么做                                                 
void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn,
                            IBuffer& buffer,
                            Timestamp receiveTime);

// void CoDefaultConnEstablishCallback(const CoTcpConnectionPtr& conn);

} // namespace net

} // namespace burger



#endif // CALLBACKS_H