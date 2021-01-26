#ifndef SOCKETSOPS_H
#define SOCKETSOPS_H

#include <arpa/inet.h>
#include <unistd.h>

namespace burger {
namespace net {
namespace sockets {
ssize_t write(int sockfd, const void *buf, size_t count);
ssize_t read(int sockfd, void *buf, size_t count);

} // namespace sockets
} // namespace net

    
} // namespace burger 


#endif // SOCKETSOPS_H