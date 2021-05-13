#ifndef COTCPSERVER_H
#define COTCPSERVER_H

#include <map>
#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>
#include "burger/base/Atomic.h"
#include "InetAddress.h"
#include "CoTcpConnection.h"
#include "Callbacks.h"

namespace burger { 
namespace net {
class InetAddress;
class Scheduler;
class Socket;

class CoTcpServer : boost::noncopyable {
public:
    using ConnectionHandler = std::function<void (const CoTcpConnection::ptr&)>;

    CoTcpServer(Scheduler* sched, const InetAddress& listenAddr, 
                            const std::string& name = "CoTcpServer", bool reuseport = true);

    ~CoTcpServer();

    void start();
    void setConnectionHandler(const ConnectionHandler& handler);
    Scheduler* getScheduler() { return sched_; }
    // void setConnEstablishCallback(const CoConnEstablishCallback& cb) { connEstablishCallback_ = cb; }
private:
    void startAccept();

private:
    Scheduler* sched_;
    InetAddress listenAddr_; 
    std::unique_ptr<Socket> listenSock_;    
    ConnectionHandler connHandler_;  
    AtomicInt32 started_;
    const std::string hostIpPort_;
    const std::string hostName_;
    int nextConnId_;
    // CoConnEstablishCallback connEstablishCallback_;   // no need this
};



} // namespace net

} // namespace burger



#endif // COTCPSERVER_H