#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <boost/noncopyable.hpp>
#include <memory>

namespace burger {
namespace net {

class TcpConnection : boost::noncopyable {
public:
    TcpConnection() = default;
    ~TcpConnection() = default;

};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
} // namespace net

} // namespace burger


#endif // TCPCONNECTION_H