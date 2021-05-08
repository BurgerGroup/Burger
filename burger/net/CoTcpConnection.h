#ifndef COTCPCONNECTION_H
#define COTCPCONNECTION_H

#include <boost/noncopyable.hpp>
#include <memory>
#include "InetAddress.h"
#include "RingBuffer.h"
#include "Callbacks.h"

namespace burger {
namespace net {

class IBuffer;
class Socket;

// todo : 写成继承的形式

class CoTcpConnection : boost::noncopyable {
public:
    using ptr = std::shared_ptr<CoTcpConnection>;
    CoTcpConnection(int sockfd, 
            const InetAddress& localAddr,
            const InetAddress& peerAddr,
            const std::string& connName);
    ~CoTcpConnection();
    // todo : more operation
    ssize_t recv(RingBuffer::ptr buf);
    ssize_t send(RingBuffer::ptr buf);
    ssize_t send(const std::string& msg);
    ssize_t send(RingBuffer::ptr buf, size_t sendSize);
    const InetAddress& getLocalAddress() const { return localAddr_; }
    const InetAddress& getPeerAddr() const { return peerAddr_; }
    const std::string& getName() const { return connName_; }
    
    void shutdown();
    // void close();
private:
    std::unique_ptr<Socket> socket_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;
    const std::string connName_;
    bool quit_;
    // todo : 需要重新设计
    // RingBuffer inputBuffer_;
    // RingBuffer outputBuffer_;

};



} // namespace net

} // namespace burger






#endif // COTCPCONNECTION_H