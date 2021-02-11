#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <functional>
#include <memory>

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
using MessageCallback = std::function<void (const TcpConnectionPtr&,
                                                    Buffer*,
                                                    Timestamp)>;

} // namespace net

} // namespace burger



#endif // CALLBACKS_H