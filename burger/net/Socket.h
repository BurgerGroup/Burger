// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

// Taken from Muduo and modified

#ifndef SOCKET_H
#define SOCKET_H
#include <boost/noncopyable.hpp>
#include <string>
#include <sstream>
#include <netinet/tcp.h>
#include "SocketsOps.h"
#include "burger/base/Log.h"
#include <cstring>
/**
 * @brief RAII封装socket file discriminator
*/

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace burger {
// tcp networking
namespace net {
class InetAddress;

class Socket : boost::noncopyable {
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}
    Socket(Socket&& sock);
    ~Socket();

    int getFd() const { return sockfd_; }
    bool getTcpinfo(struct tcp_info& tcpinfo) const;
    std::string getTcpInfoString() const;

    // abort if address in use
    void bindAddress(const InetAddress& localaddr);
    // abort if address in use
    void listen();
    int accept(InetAddress& peeraddr);

    void shutdownWrite();
    // Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
    
private:
    const int sockfd_;
};


} // namespace net

} // namespace burger


#endif // SOCKET_H