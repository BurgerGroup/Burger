#ifndef SOCKETSOPS_H
#define SOCKETSOPS_H

#include <arpa/inet.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>

namespace burger {
namespace net {
namespace sockets {
ssize_t write(int sockfd, const void *buf, size_t count);
ssize_t read(int sockfd, void *buf, size_t count);

void ipPortToAddrin(const std::string& ip, uint16_t port, 
                        struct sockaddr_in* addr);

const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);

std::string toIp(const struct sockaddr_in* addr);

} // namespace sockets
} // namespace net

    
} // namespace burger 


#endif // SOCKETSOPS_H