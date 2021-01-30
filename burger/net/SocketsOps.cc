#include "SocketsOps.h"
#include "Endian.h"
#include "burger/base/Log.h"
#include "burger/base/Util.h"
using namespace burger;
using namespace burger::net;

// TODO: wrap error?
ssize_t sockets::write(int sockfd, const void *buf, size_t count) {
    return ::write(sockfd, buf, count);
}

ssize_t sockets::read(int sockfd, void *buf, size_t count) {
    return ::read(sockfd, buf, count);
}

void sockets::ipPortToAddrin(const std::string& ip, uint16_t port, 
                        struct sockaddr_in* addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port);
    if(::inet_pton(AF_INET, ip.c_str(), &addr->sin_addr) <= 0) {
        ERROR("sockets::ipPortToAddrin");
    }
}

// TODO
const struct sockaddr_in* sockets::sockaddr_in_cast(const struct sockaddr* addr) {
    return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

std::string toIp(const struct sockaddr_in* addr) {
    char buf[32];
    ::inet_ntop(AF_INET, &addr->sin_addr, buf, static_cast<socklen_t>(sizeof(buf)));
    return buf;
}
