#ifndef DISCARD_H
#define DISCARD_H

#include "burger/net/CoTcpServer.h"
#include "burger/base/Log.h"
using namespace burger;
using namespace burger::net;

// RFC 836
// 最简单的长连接TCP应用层协议
class DiscardServer {
public:
    DiscardServer(Scheduler* sched, const InetAddress& listenAddr);
    void start();
private:
    void connHandler(const CoTcpConnection::ptr& conn);
    CoTcpServer server_;
};


#endif // DISCARD_H