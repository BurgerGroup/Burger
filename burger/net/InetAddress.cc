#include "InetAddress.h"
#include "Endian.h"
#include "SocketsOps.h"
#include <netdb.h>
#include <cstddef>
#include <cstring>
// INADDR_ANY use (type)value casting.
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
#pragma GCC diagnostic error "-Wold-style-cast"

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

using namespace burger;
using namespace burger::net;

// https://www.cnblogs.com/xiongxinxzy/p/13786047.html
static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in), 
                "InetAddress is same size as sockaddr_in");
static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");

InetAddress::InetAddress(uint16_t port) {
    static_assert(offsetof(InetAddress, addr_) == 0, "addr_ offset 0");
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = sockets::hostToNetwork32(kInaddrAny);
    addr_.sin_port = sockets::hostToNetwork16(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port) {
    bzero(&addr_, sizeof(addr_));
    sockets::ipPortToAddrin(ip, port, &addr_);
}

InetAddress::InetAddress(const struct sockaddr_in& addr):
    addr_(addr){
}

std::string InetAddress::getIpStr() const {
    char buf[32] = "";
    return sockets::toIp(&addr_);
}

std::string InetAddress::getPortStr() const {
    return std::to_string(getPort());
}






