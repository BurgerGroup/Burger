#ifndef ECHO_H
#define ECHO_H

#include "burger/net/CoTcpServer.h"
#include "burger/base/Atomic.h"
#include "burger/base/Log.h"

using namespace burger;
using namespace burger::net;


class EchoServer {
public:
    EchoServer(Scheduler* sched, const InetAddress& listenAddr, int maxConnections);
    void start();  
private:
    void connHandler(const CoTcpConnection::ptr& conn);
private:    
    AtomicInt32 numConnected_;   // 当前连接数
    CoTcpServer server_;
    const int kMaxConnections_;   // 允许连接最大数目
};

#endif // ECHO_H