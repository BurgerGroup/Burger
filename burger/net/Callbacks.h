#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <functional>
#include <memory>
#include "burger/base/Timestamp.h"

namespace burger {
namespace net {

// All client visible callbacks go here.

class TcpConnection;
class Buffer;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TimerCallback = std::function<void()> ;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void (const TcpConnectionPtr&)>;
using HighWaterMarkCallback = std::function<void (const TcpConnectionPtr&, size_t)>;
using MessageCallback = std::function<void (const TcpConnectionPtr&,
                                                    Buffer&,
                                                    Timestamp)>;
// 声明于此，实现在Tcpconnection里面, 然后在Tcpserver里设置
// TODO : 为何这么做                                                 
void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn,
                            Buffer& buffer,
                            Timestamp receiveTime);

} // namespace net

} // namespace burger



#endif // CALLBACKS_H