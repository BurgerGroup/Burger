#ifndef CHARGEN_H
#define CHARGEN_H

#include "burger/net/TcpServer.h"
#include <string>

using namespace burger;
using namespace burger::net;
// RFC 864
// chargen服务器很特殊，只发送数据，不接收数据
// 而且它发送的数据不能快过客户端接收的速度
// onWriteComplete
class ChargenServer {
public:
    ChargenServer(EventLoop* loop,
                const InetAddress& listenAddr,
                bool print = false);
    void start();

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn,
                   Buffer& buf,
                   Timestamp time);
    void onWriteComplete(const TcpConnectionPtr& conn);
    void printThroughput();

private:
    TcpServer server_;

    std::string message_;
    int64_t transferred_;
    Timestamp startTime_;
};




#endif // CHARGEN_H