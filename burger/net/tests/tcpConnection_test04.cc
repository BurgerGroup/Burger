#include "burger/net/TcpServer.h"
#include "burger/net/EventLoop.h"
#include "burger/net/InetAddress.h"
#include <iostream>

using namespace burger;
using namespace burger::net;
/**
 * SIGPIPE 
 * SIGPIPE 默认行为是终止进程，如果对方断开连接而本地继续写入的话，会造成服务进程的意外退出
 * 加入服务进程繁忙，没有及时处理对方断开连接的事件，就有可能出现在连接断开后继续发送数据的情况
 * 
 * 模拟 : nc localhost后立刻ctrl-c断开客户端，服务进程过几秒就会退出
 * 在EventLoop.cc中 IgonoreSigPipe则解决问题
 */

std::string message1;
std::string message2;
int sleepSeconds = 0;

void onConnection(const TcpConnectionPtr& conn) {
    if(conn->isConnected()) {
        std::cout << "onConnection(): new connection [" 
            << conn->getName() <<  "] from " 
            << conn->getPeerAddress().getIpPortStr() << std::endl;
        if(sleepSeconds > 0) {
            ::sleep(sleepSeconds);
        }
        conn->send(message1);
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
    if(argc > 3) {
        sleepSeconds = atoi(argv[3]);
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