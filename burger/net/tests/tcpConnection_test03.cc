#include "burger/net/TcpServer.h"
#include "burger/net/EventLoop.h"
#include "burger/net/InetAddress.h"
#include <iostream>

using namespace burger;
using namespace burger::net;

/**
 * @brief 通过命令行指定两条消息大小，连续发送两条消息 
 * sendInLoop和handleWrite都只调用了一次write而不是反复调用直到返回EAGAIN
 * reason: 如果第一次write没有能够发送完数据，第二次调用write几乎肯定返回EAGAIN
 * 对比见 tcpConnection_test03.py
 * EAGAIN is often raised when performing non-blocking I/O. It means "there is no data available right now, try again later".
 * It might (or might not) be the same as EWOULDBLOCK, which means "your thread would have to block in order to do that".
 * 
 * https://stackoverflow.com/questions/4058368/what-does-eagain-mean
 */

std::string message1;
std::string message2;

void onConnection(const TcpConnectionPtr& conn) {
    if(conn->isConnected()) {
        std::cout << "onConnection(): new connection [" 
            << conn->getName() <<  "] from " 
            << conn->getPeerAddress().getIpPortStr() << std::endl;
        // [EventLoop.cc : 244 <printActiveChannels>] [ 9: IN HUP  ]
        // 当服务器端主动断开与客户端的连接，这意味着客户端read返回0, close(connfd),
        // 服务器端收到的事件为 POLLHUP | POLLIN

        // Muduo 这种关闭连接的方式对对方也有要求，那就是对方 read() 到 0 字节之后会主动关闭连接（无论 shutdownWrite() 还是 close()），
        // 一般的网络程序都会这样，不是什么问题。
        // 当然，这么做有一个潜在的安全漏洞，万一对方故意不不关，那么 muduo 的连接就一直半开着，消耗系统资源。
        // https://blog.csdn.net/Solstice/article/details/6208634
        conn->send(message2);
        conn->shutdown();
    } else {
        std::cout << "onConnection() : connection " 
            << conn->getName() << " is down" << std::endl;
    }   
}

void onMessage(const TcpConnectionPtr& conn, 
                Buffer& buf, 
                Timestamp recieveTime) {
    std::cout << "onMessage(): received " << buf.getReadableBytes() 
        << " bytes from connection " << conn->getName() 
        << " at " << recieveTime.toFormatTime() <<std::endl;
    buf.retrieveAll();
}   

int main(int argc, char *argv[]) {

    std::cout << "main() : pid = " << ::getpid() << std::endl;
    int len1 = 100;
    int len2 = 200;
    if(argc > 2) {
        len1 = atoi(argv[1]);
        len2 = atoi(argv[2]);
    }
    message1.resize(len1);
    message2.resize(len2);
    std::fill(message1.begin(), message1.end(), 'A');
    std::fill(message2.begin(), message2.end(), 'B');

    InetAddress listenAddr(8888);
    EventLoop loop;
    TcpServer server(&loop, listenAddr, "TcpServer");
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.loop();
}