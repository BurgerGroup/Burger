#include "chargen.h"
#include "burger/base/Log.h"
#include "burger/net/Scheduler.h"
#include <stdio.h>

using namespace std::placeholders;

ChargenServer::ChargenServer(Scheduler* sched, const InetAddress& listenAddr)
    : server_(sched, listenAddr, "Chargen Server"),
    transferred_(0),
    startTime_(Timestamp::now()) {
    server_.setConnectionHandler(std::bind(&ChargenServer::connHandler, this, _1));
    TRACE("ChargenServer created");
    std::string line;
    // 33 - 126的可打印ascii
    for(int i = 33; i < 127; i++) {
        line.push_back(char(i));
    }
    // 33 34 ... 126 33 34 .. 126
    line += line;
    // 滑动窗口一样的操作，每排72个，一共94排
    for(size_t i = 0; i < 127-33; i++) {
        message_ += line.substr(i, 72) + '\n';
    }
    
}

void ChargenServer::start() {
    server_.start();
    // runEvery 等要在start后才能执行，之前reactor模式是先建立好
    server_.getScheduler()->runEvery(3.0, std::bind(&ChargenServer::printThroughput, this));
}

void ChargenServer::connHandler(const CoTcpConnection::ptr& conn) {
    conn->setTcpNoDelay(true); // 设置true为了测试吞吐量  有数据就立刻发送，不需要等待
    while(conn->isConnected()) {
        conn->send(message_);  // 然后发送94行的数据过去
        transferred_ += message_.size();
    }
}

void ChargenServer::printThroughput() {
    Timestamp endTime = Timestamp::now();
    double time = timeDifference(endTime, startTime_);
    printf("%4.3f MiB/s\n", static_cast<double>(transferred_)/time/1024/1024);
    transferred_ = 0;
    startTime_ = endTime;
}
