#include "burger/net/Acceptor.h"
#include "burger/net/InetAddress.h"
#include "burger/net/EventLoop.h"
#include <iostream>

// telnet 127.0.0.1 8888
using namespace burger;
using namespace burger::net;

void newConnection(int sockfd, const InetAddress& peerAddr) {
    std::cout << "newConnection() : accepted a new connection from "
        << peerAddr.getIpPortStr() << std::endl;
    sockets::write(sockfd, "How are you?\n");
    sockets::close(sockfd);
}

int main() {
    if (!Logger::Instance().init("log", "logs/test.log", spdlog::level::trace)) {
        std::cout << "Logger init error" << std::endl;
		return 1;
	}

    std::cout << "main() : pid = " << ::getpid() << std::endl;
    InetAddress listenAddr(8888);
    EventLoop loop;
    
    Acceptor acceptor(&loop, listenAddr);
    acceptor.setNewConnectionCallback(newConnection);
    acceptor.listen();

    loop.loop();
}