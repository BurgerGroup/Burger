#ifndef SOCKET_H
#define SOCKET_H

/**
 * @brief RAII封装socket file discriminator
*/

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace burger {
// tcp networking
namespace net {
class InetAddress;



} // namespace net

} // namespace burger


#endif // SOCKET_H