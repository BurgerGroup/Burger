#ifndef CHARGEN_H
#define CHARGEN_H

#include "burger/net/TcpServer.h"
#include <string>

using namespace burger;
using namespace burger::net;
// RFC 864
// 如果没setHighWaterMarkCallback, 一次发很大的流量，然后0 ， 0， 0

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