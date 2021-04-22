#ifndef HOOK_H
#define HOOK_H

#include <sys/socket.h>
#include "burger/base/Coroutine.h"

namespace burger {
namespace net {

bool isHookEnable();
void setHookEnabled(bool flag);

} // namespace net
} // namespace burger


extern "C" {

//sleep
typedef unsigned int (*sleep_t)(unsigned int seconds);
extern sleep_t sleep_f;

//accept
typedef int (*accept_t)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
extern accept_t accept_f;

//accept4
typedef int (*accept4_t)(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);
extern accept4_t accept4_f;

//connect
typedef int (*connect_t)(int sockfd, const struct sockaddr *addr,
				                   socklen_t addrlen);
extern connect_t connect_f;

//read
typedef ssize_t (*read_t)(int fd, void *buf, size_t count);
extern read_t read_f;

//readv
typedef ssize_t (*readv_t)(int fd, const struct iovec *iov, int iovcnt);
extern readv_t readv_f;

//recv
typedef ssize_t (*recv_t)(int sockfd, void *buf, size_t len, int flags);
extern recv_t recv_f;

//recvfrom
typedef ssize_t (*recvfrom_t)(int sockfd, void *buf, size_t len, int flags, 
				struct sockaddr *src_addr, socklen_t *addrlen);
extern recvfrom_t recvfrom_f;

//recvmsg
typedef ssize_t (*recvmsg_t)(int sockfd, struct msghdr *msg, int flags);
extern recvmsg_t recvmsg_f;

//write
typedef ssize_t (*write_t)(int fd, const void *buf, size_t count);
extern write_t write_f;

//writev
typedef ssize_t (*writev_t)(int fd, const struct iovec *iov, int iovcnt);
extern writev_t writev_f;

//send
typedef ssize_t (*send_t)(int sockfd, const void *buf, size_t len, int flags);
extern send_t send_f;

//sendto
typedef ssize_t (*sendto_t)(int sockfd, const void *buf, size_t len, int flags, 
			const struct sockaddr *dest_addr, socklen_t addrlen);
extern sendto_t sendto_f;

//sendmsg
typedef ssize_t (*sendmsg_t)(int sockfd, const struct msghdr *msg, int flags);
extern sendmsg_t sendmsg_f;

}







#endif // HOOK_H