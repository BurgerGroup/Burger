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
// 这些步骤出错就会造成程序终止，通常原因是因为端口被占用，这时让程序异常更好的退出，触发监控系统警报，而不是假装正常运行
int createNonblockingOrDie();
int connect(int sockfd, const struct sockaddr& addr);
void bindOrDie(int sockfd, const struct sockaddr_in& addrin);
void listenOrDie(int sockfd);
int accept(int sockfd, struct sockaddr_in& addrin);

ssize_t write(int sockfd, const std::string& msg);

ssize_t write(int sockfd, const void *buf, size_t count);
ssize_t read(int sockfd, void *buf, size_t count);
ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);

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
struct sockaddr_in getPeerAddr(int sockfd);
bool isSelfConnect(int sockfd);

} // namespace sockets
} // namespace net

    
} // namespace burger 


#endif // SOCKETSOPS_H