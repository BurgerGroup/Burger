#ifndef DAYTIME_H
#define DAYTIME_H

#include "burger/net/CoTcpServer.h"

using namespace burger;
using namespace burger::net;

// daytime 是短连接协议, 在发送完当前时间后，由服务端主动断开连接
// RFC 867
class DaytimeServer {
public: 
    DaytimeServer(Scheduler* sched, const InetAddress& listenAddr);
    void start();
private:
    void connHandler(const CoTcpConnection::ptr& conn);
private: 
    CoTcpServer server_;
};


#endif // DAYTIME_H