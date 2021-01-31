#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <functional>
#include <memory>

namespace burger {
namespace net {

// All client visible callbacks go here.

class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TimerCallback = std::function<void()> ;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;

// tmp
using MessageCallback = std::function<void(const TcpConnectionPtr&,
                                            const char* data,
                                            ssize_t len)>;

} // namespace net

} // namespace burger



#endif // CALLBACKS_H