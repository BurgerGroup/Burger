#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>
#include "burger/base/Timestamp.h"

namespace burger {
namespace net {

class InetAddress;
class EventLoop;
class Channel;
class Socket;

class Acceptor : boost::noncopyable {
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport = true);
    ~Acceptor();
    void setNewConnectionCallback(const NewConnectionCallback& cb) { newConnectionCallback_ = cb; }
    void listen();
    bool isListening() const { return listening_; }
private: 
    void handleRead(Timestamp timestamp);

    EventLoop* loop_;
    std::unique_ptr<Socket> acceptSocket_;
    std::unique_ptr<Channel> acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
    int idleFd_;   // 占位fd,用于fd满的情况，避免一直电平触发
};
} // namespace net

    
} // namespace burger



#endif // ACCEPTOR_H