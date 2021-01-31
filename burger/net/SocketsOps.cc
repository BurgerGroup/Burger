#include "SocketsOps.h"
#include "Endian.h"
#include "burger/base/Log.h"
#include "burger/base/Util.h"
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
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
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

