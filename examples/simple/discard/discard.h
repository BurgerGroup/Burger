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
    DiscardServer(const InetAddress& listenAddr, int threadNum = 1, const std::string& name = "tcpserver");
    void start();
private:
    void connHandler(CoTcpConnection::ptr conn);
    CoTcpServer server_;
};


#endif // DISCARD_H