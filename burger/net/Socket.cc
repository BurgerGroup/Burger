#include "Socket.h"
#include "InetAddress.h"

using namespace burger;
using namespace burger::net;


Socket::Socket(Socket&& sock) :
    sockfd_(sock.sockfd_) {
    *(const_cast<int*>(&sock.sockfd_)) = -1;
}

Socket::~Socket() {
    sockets::close(sockfd_);
}

bool Socket::getTcpinfo(struct tcp_info& tcpinfo) const {
    socklen_t len = sizeof(tcpinfo);
    bzero(&tcpinfo, len);
    return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, &tcpinfo, &len) == 0;
}

std::string Socket::getTcpInfoString() const {
    struct tcp_info tcpinfo;
    bool ok = getTcpinfo(tcpinfo);
    std::ostringstream oss;
    if(ok) {
        // https://blog.csdn.net/dyingfair/article/details/95855952
        oss << "unrecovered = " << tcpinfo.tcpi_retransmits   //重传数，表示当前待重传的包数，这个值在重传完毕后清零
        << " rto = " << tcpinfo.tcpi_rto  //重传超时时间，这个和RTT有关系，RTT越大，rto越大
        << " ato = " << tcpinfo.tcpi_ato     //用来延时确认的估值，单位为微秒.
        << "\nsnd_mss = " <<  tcpinfo.tcpi_snd_mss // 本端的MSS
        << " rcv_mss = " << tcpinfo.tcpi_rcv_mss // 对端的MSS
        << " lost = " << tcpinfo.tcpi_lost  // 本端在发送出去被丢失的报文数。重传完成后清零
        << "\n retrans = " << tcpinfo.tcpi_retrans // 重传且未确认的数据段数
        << " rtt = " << tcpinfo.tcpi_rtt     // smoothed round trip time,微妙
        << " rttvar = " << tcpinfo.tcpi_rttvar  // 描述RTT的平均偏差，该值越大，说明RTT抖动越大
        << "\nsshthresh = " << tcpinfo.tcpi_snd_ssthresh  // 拥塞控制慢开始阈值
        << " cwnd = " << tcpinfo.tcpi_snd_cwnd // 拥塞控制窗口大小
        << " total_retrans = " << tcpinfo.tcpi_total_retrans; //统计总重传的包数，持续增长。
    }
    return oss.str();

}

void Socket::bindAddress(const InetAddress& localaddr) {
    sockets::bindOrDie(sockfd_, localaddr.getSockAddrin());
}

void Socket::listen() {
    sockets::listenOrDie(sockfd_);
}

int Socket::accept(InetAddress& peeraddr) {
    struct sockaddr_in addrin;
    bzero(&addrin, sizeof(addrin));
    int connfd = sockets::accept(sockfd_, addrin);
    if(connfd > 0) {
        peeraddr.setSockAddrin(addrin);
    }
    return connfd;
}

void Socket::shutdownWrite() {
    sockets::shutdownWrite(sockfd_);
}

// Nagle算法可以一定程度上避免网络拥塞
// TCP_NODELAY 选项可以禁用Nagle算法
// 禁用Nagle算法，可以避免连续发包出现延迟，这对于编写延迟的网络服务很重要
void Socket::setTcpNoDelay(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, 
        &opt, static_cast<socklen_t>(sizeof(opt)));
}

void Socket::setReuseAddr(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
            &opt, static_cast<socklen_t>(sizeof(opt)));
}

void Socket::setReusePort(bool on) {
#ifdef SO_REUSEPORT 
    int opt = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                &opt, static_cast<socklen_t>(sizeof(opt)));
    if(ret < 0 && on) {
        ERROR("SO_REUSEPORT failed.");
    }
#else 
    if(on) {
        ERROR("SO_REUSEPORT is not supported.");
    }
#endif 
}

// 定期探测连接是否存在，如果应用层有心跳的话，这个选项不是必需要设置的
void Socket::setKeepAlive(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
                &opt, static_cast<socklen_t>(sizeof(opt)));
}
















