#include "burger/net/CoTcpServer.h"
#include "burger/base/Log.h"
#include "burger/net/RingBuffer.h"
#include <string>
#include <iostream>

using namespace burger;
using namespace burger::net;

void connHandler(CoTcpConnection::ptr conn) {
    const std::string msg = "hello!";
    int cnt = 0;
    while(cnt < 20) {
        std::string tmp = msg + std::to_string(cnt);
        conn->send(std::move(tmp));
        ++cnt;
        if(cnt == 10) {
            conn->shutdown();
            std::cout << "when conn is shut down, cnt is " << cnt << std::endl;
        }
    }
    std::cout << "when taks is over, cnt is " << cnt << std::endl;
}

int main() {
    LOGGER(); LOG_LEVEL_TRACE;
    CoTcpServer server(8888, 2);
    server.setConnectionHandler(connHandler);
    server.start();
    return 0;
}