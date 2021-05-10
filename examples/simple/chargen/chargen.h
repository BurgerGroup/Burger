#ifndef CHARGEN_H
#define CHARGEN_H

#include "burger/base/Timestamp.h"
#include "burger/net/CoTcpServer.h"
#include <string>

using namespace burger;
using namespace burger::net;
// RFC 864
// chargen服务器很特殊，只发送数据，不接收数据
// TCP连接建立后，服务器不断传送任意的字符到客户端，直到客户端关闭
// ASCII 32 - 125 为可打印字符
// 33 34 35 ... 104   
// 34 35 36 ... 105
// ...
// 126 33 34 35 ... 103
// 每一行都是72个字符, 一共94行 
// 然后
// 而且它发送的数据不能快过客户端接收的速度
// todo : should we really need onWriteComplete ?  

class ChargenServer {
public:
    ChargenServer(Scheduler* sched, const InetAddress& listenAddr);
    void start();

private:
    void connHandler(const CoTcpConnection::ptr& conn);
    void printThroughput();

private:
    CoTcpServer server_;

    std::string message_;
    int64_t transferred_;
    Timestamp startTime_;
};

#endif // CHARGEN_H