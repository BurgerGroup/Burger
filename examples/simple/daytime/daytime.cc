#include "daytime.h"

using namespace std::placeholders;

DaytimeServer::DaytimeServer(Scheduler* sched, const InetAddress& listenAddr)
    : server_(sched, listenAddr, "DayTimeServer") {
    server_.setConnectionHandler(std::bind(&DaytimeServer::connHandler, this, _1));
}

void DaytimeServer::start() {
    server_.start();
}

void DaytimeServer::connHandler(const CoTcpConnection::ptr& conn) {
    conn->send(Timestamp::now().toFormatTime() + "\n");
    conn->shutdown();
    // 此处shutdown只关闭写端
}






