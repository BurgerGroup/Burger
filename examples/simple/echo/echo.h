#ifndef ECHO_H
#define ECHO_H

#include "burger/net/CoTcpServer.h"
#include "burger/base/Log.h"

using namespace burger;
using namespace burger::net;

// RFC 862
// 回显服务器，把收到的数据发回客户端
// 是一个双向协议
// 这个代码不是行回显(line echo)服务，而是有一点数据发一点，这样可以避免客户端恶意发来不换行字符
// 服务端要缓存已经收到的数据，导致服务器内存爆涨
// 此处有个安全漏洞，客户端故意发送数据，但是从不接收，服务端发送缓冲区一直堆积，导致内存暴涨
// 解决办法可以参考chargen协议 or 发送缓冲区累积到一定大小主动断开连接
// todo : 我们这里应该不会堆积把
class EchoServer {
public:
    EchoServer(Scheduler* sched, const InetAddress& listenAddr);
    void start();  
private:
    void connHandler(const CoTcpConnection::ptr& conn);
private:    
    CoTcpServer server_;
};

#endif // ECHO_H