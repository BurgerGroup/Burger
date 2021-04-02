#ifndef ECHO_H
#define ECHO_H

#include "burger/net/TcpServer.h"
#include "burger/base/Timestamp.h"
#include "burger/net/InetAddress.h"

using namespace burger;
using namespace burger::net;

// RFC 862
// 回显服务器，把收到的数据发回客户端
class EchoServer {
public:
    EchoServer(EventLoop* loop, const InetAddress& listenAddr);\
    void start();  // calls server_.start();
private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer& buf, Timestamp time);

private:    
    TcpServer server_;
};

#endif // ECHO_H