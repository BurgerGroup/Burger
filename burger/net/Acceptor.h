#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <boost/noncopyable.hpp>
#include <functional>


namespace burger {
namespace net {

class Acceptor : boost::noncopyable {
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>
private:

}
} // namespace net

    
} // namespace burger



#endif // ACCEPTOR_H