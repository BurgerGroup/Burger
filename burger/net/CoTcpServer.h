#ifndef COTCPSERVER_H
#define COTCPSERVER_H

#include <vector>
#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>
#include "burger/base/Atomic.h"
#include "InetAddress.h"
#include "CoTcpConnection.h"
namespace burger { 
namespace net {
class InetAddress;
class Scheduler;
class Socket;


class CoTcpServer : boost::noncopyable {
public:
    using ConnectionHandler = std::function<void (CoTcpConnection::ptr)>;
    CoTcpServer(const InetAddress& listenAddr, int threadNum = 1, const std::string& name = "tcpserver");
    ~CoTcpServer();

    void start();
    void setConnectionHandler(const ConnectionHandler& handler);
private:
    void startAccept();

private:
    InetAddress listenAddr_; // really need this?
    std::unique_ptr<Socket> listenSock_;    
    std::unique_ptr<Scheduler> sched_;
    ConnectionHandler connHandler_;
    AtomicInt32 started_;
    const std::string hostIpPort_;
    const std::string hostName_;
    int nextConnId_;
};



} // namespace net

} // namespace burger



#endif // COTCPSERVER_H