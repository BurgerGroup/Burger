#include "SocketsOps.h"
#include "Endian.h"
#include "burger/base/Log.h"
#include "burger/base/Util.h"
#include <sys/uio.h>  // readv
using namespace burger;
using namespace burger::net;

#if VALGRIND || defined (NO_ACCEPT4)
void setNonBlockAndCloseOnExec(int sockfd) {
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;    
    int res = ::fcntl(sockfd, F_SETFL, flags);

    flags = ::fcntl(sockfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(sockfd, F_SETFD, flags);
}

#endif

int sockets::createNonblockingOrDie() {
#if VALGRIND
    int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd < 0) {
        CRITICAL("sockets::createNonblockingOrDie");
    }
    setNonBlockAndCloseOnExec(sockfd);
#else 
    // 直接在创建的时候指定非阻塞和exec关闭选项
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC
                        , IPPROTO_TCP);
    if(sockfd < 0) {
        CRITICAL("sockets::createNonblockingOrDie");
    }
#endif 
    return sockfd;
}

int sockets::connect(int sockfd, const struct sockaddr& addr) {
    return ::connect(sockfd, &addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
}

void sockets::bindOrDie(int sockfd, const struct sockaddr_in& addrin) {
    int ret = ::bind(sockfd, 
                sockaddr_cast(&addrin), 
                static_cast<socklen_t>(sizeof(addrin)));
    if(ret < 0) {
        CRITICAL("sockets::bindOrDie");
    }
}

void sockets::listenOrDie(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);
    if(ret < 0) {
        CRITICAL("sockets::listenOrDie");
    } 
}

int sockets::accept(int sockfd, struct sockaddr_in& addrin) {
    socklen_t addrlen = static_cast<socklen_t>(sizeof(addrin));
#if VALGRIND || defined (NO_ACCEPT4)
    int connfd = ::accept(sockfd, sockaddr_cast(addrin), &addrlen);
    setNonBlockAndCloseOnExec(connfd);
#else 
    // 内核较新可用此
    int connfd = ::accept4(sockfd, sockaddr_cast(&addrin),
                    &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif 
    if(connfd < 0) {
        int savedErrno = errno;
        ERROR("socket::accept");
        switch (savedErrno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = savedErrno;  // 暂时错误，处理办法是忽略这次错误
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:  
            case ENFILE:
            case ENOBUFS:        
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:   // 致命错误和未知错误 终止程序
                // unexpected errors
                CRITICAL("unexpected error of ::accept {}", savedErrno);
                break;
            default:
                CRITICAL("unknown error of ::accept {}", savedErrno);
                break;
        }
    }
    return connfd;
}

// TODO : CHECK
ssize_t sockets::write(int sockfd, const std::string& msg) {
    return ::write(sockfd, msg.c_str(), msg.length());
}

// TODO: wrap error?
ssize_t sockets::write(int sockfd, const void *buf, size_t count) {
    return ::write(sockfd, buf, count);
}

// ssize_t sockets::writen(int sockfd, const void* buf, size_t count) {
// 	size_t writen = 0;
// 	size_t left = count;
// 	while (left > 0) {
// 		ssize_t n = ::write(static_cast<const char*>(buf) + writen, left);
// 		if (n <= 0) {
// 			return writen;
// 		}
// 		writen += n;
// 		left -= n;
// 	}
// 	return count;
// }

ssize_t sockets::read(int sockfd, void *buf, size_t count) {
    return ::read(sockfd, buf, count);
}

ssize_t sockets::readv(int sockfd, const struct iovec *iov, int iovcnt) {
    return ::readv(sockfd, iov, iovcnt);
}
// ssize_t sockets::readn(int sockfd, void* buf, size_t count) {
// 	size_t readn = 0;
// 	size_t left = count;
// 	while (left > 0) {
// 		ssize_t n = read(static_cast<char*>(buf) + readn, left);
// 		if (n <= 0) {
// 			return readn;
// 		}
// 		readn += n;
// 		left -= n;
// 	}
// 	return count;
// }

void sockets::ipPortToAddrin(const std::string& ip, uint16_t port, 
                        struct sockaddr_in* addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port);
    if(::inet_pton(AF_INET, ip.c_str(), &addr->sin_addr) <= 0) {
        ERROR("sockets::ipPortToAddrin");
    }
}

int sockets::getSocketError(int sockfd) {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
    if(::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
}


const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in* addr) {
    return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in* addr) {
    return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr_in* sockets::sockaddr_in_cast(const struct sockaddr* addr) {
    return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

struct sockaddr_in* sockets::sockaddr_in_cast(struct sockaddr* addr) {
    return static_cast<struct sockaddr_in*>(implicit_cast<void*>(addr));
}

std::string sockets::toIpStr(const struct sockaddr_in* addr) {
    char buf[32];
    ::inet_ntop(AF_INET, &addr->sin_addr, buf, static_cast<socklen_t>(sizeof(buf)));
    return buf;
}

void sockets::close(int sockfd) {
    if(::close(sockfd) < 0) {
        ERROR("sockets::close");
    }
}

void sockets::shutdownWrite(int sockfd) {
    if(::shutdown(sockfd, SHUT_WR) < 0) {
        ERROR("sockets::shutdownWrite");
    }
}

struct sockaddr_in sockets::getLocalAddr(int sockfd) {
    struct sockaddr_in localaddr;
    bzero(&localaddr, sizeof(localaddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(localaddr));
    if(::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
        ERROR("sockets::getLocalAddr");
    }
    return localaddr;
}

struct sockaddr_in sockets::getPeerAddr(int sockfd) {
    struct sockaddr_in peeraddr;
    bzero(&peeraddr, sizeof peeraddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
    if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0) {
        ERROR("sockets::getPeerAddr");
    }
    return peeraddr;
}

bool sockets::isSelfConnect(int sockfd) {
    struct sockaddr_in localaddr = getLocalAddr(sockfd);
    struct sockaddr_in peeraddr = getPeerAddr(sockfd);
    return localaddr.sin_port == peeraddr.sin_port
        && localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}

// create a file descriptor for event notification
// 主要是用于进程或者线程间的通信(如通知/等待机制的实现)。
int sockets::createEventfd() {
    int efd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(efd < 0) {
        ERROR("Failed in eventfd");
        abort();
    }
    return efd;
}
