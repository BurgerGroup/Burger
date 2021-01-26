#include "SocketsOps.h"

using namespace burger;
using namespace burger::net;

// TODO: wrap error?
ssize_t sockets::write(int sockfd, const void *buf, size_t count) {
    return ::write(sockfd, buf, count);
}

ssize_t sockets::read(int sockfd, void *buf, size_t count) {
    return ::read(sockfd, buf, count);
}

