#include "TcpClient.h"
#include "EventLoop.h"

using namespace burger;
using namespace burger::net;


namespace burger {
namespace net {
namespace detail {

void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn) {
    loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr& connector) {
    //connector->
}

}  // namespace detail
}  // namespace net
}  // namespace muduo

TcpClient::TcpClient(EventLoop* loop,
                const InetAddress& serverAddr,
                const std::string& name):
        loop_(loop),
        connector_(std::make_shared<Connector>(loop, serverAddr)),
        name_(name),
        connectionCallback_(defaultConnectionCallback),
        messageCallback_(defaultMessageCallback),
        retry_(false),
        connect_(true),
        nextConnId_(1) {
    connector_->setNewConnectionCallback(std::bind(&TcpClient::newConnection, this,
                            std::placeholders::_1));
    // FIXME setConnectFailedCallback
    INFO("TcpClient::TcpClient[{}] - connector {}", name_, fmt.ptr(connector_));
}

TcpClient::~TcpClient() {
    INFO("TcpClient::~TcpClient[{}] - connector {}", name_, fmt.ptr(connector_));

    TcpConnectionPtr conn;
    bool unique = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        unique = connection_.unique();
        conn = connection_;
    }
    if (conn) {
        assert(loop_ == conn->getLoop());
        // FIXME: not 100% safe, if we are in different thread
        CloseCallback cb = std::bind(&detail::removeConnection, loop_, 
                                        std::placeholders::_1);
        loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
        if (unique) {
            conn->forceClose();
        }
    } else {
        connector_->stop();
        // FIXME: HACK
        loop_->runAfter(1, std::bind(&detail::removeConnector, connector_));
    }
}

void TcpClient::connect() {
    INFO("TcpClient::connect[{}] - connecting to {}", 
            name_, connector_->getServerAddress().getIpPortStr());
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect() {
    connect_ = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (connection_) {
            connection_->shutdown();
        }
    }
}

void TcpClient::stop() {
    connect_ = false;
    connector_->stop();
}

TcpConnectionPtr TcpClient::connection() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return connection_;
}

void TcpClient::newConnection(int sockfd) {
    loop_->assertInLoopThread();
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    std::string connName = name_ + ":" + peerAddr.getIpPortStr() + "#" + std::to_string(nextConnId_);
    ++nextConnId_;
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    TcpConnectionPtr conn(std::make_shared<TcpConnection>(loop_,
                    connName, sockfd, localAddr, peerAddr));
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
    std::bind(&TcpClient::removeConnection, this, std::placeholders::_1)); // FIXME: unsafe
    {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn) {
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());
    {
        std::lock_guard<std::mutex> lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }
    loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    if (retry_ && connect_) {
        INFO("TcpClient::connect[{}] - Reconnecting to {}", 
                    name_, connector_->getServerAddress().getIpPortStr());
        connector_->restart();
    }
}





