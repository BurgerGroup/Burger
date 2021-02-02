#ifndef SOCKETSOPS_H
#define SOCKETSOPS_H

#include <arpa/inet.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>

namespace burger {
namespace net {
namespace sockets {
// Creates a non-blocking socket fd, abort if any error
int createNonblockingOrDie();
void bindOrDie(int sockfd, const struct sockaddr_in& addrin);
void listenOrDie(int sockfd);
int accept(int sockfd, struct sockaddr_in& addrin);

ssize_t write(int sockfd, std::string msg);

ssize_t write(int sockfd, const void *buf, size_t count);
ssize_t read(int sockfd, void *buf, size_t count);

void close(int sockfd);
void shutdownWrite(int sockfd);
void ipPortToAddrin(const std::string& ip, uint16_t port, 
                        struct sockaddr_in* addr);

int getSocketError(int sockfd);

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
struct sockaddr_in* sockaddr_in_cast(struct sockaddr* addr);

std::string toIpStr(const struct sockaddr_in* addr);

struct sockaddr_in getLocalAddr(int sockfd);


} // namespace sockets
} // namespace net

    
} // namespace burger 


#endif // SOCKETSOPS_H