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
    // 连接成功回调函数
    connector_->setNewConnectionCallback(std::bind(&TcpClient::newConnection, this,
                            std::placeholders::_1)); // sockfd
    // FIXME setConnectFailedCallback
    INFO("TcpClient::TcpClient[{}] - connector {}", name_, fmt::ptr(connector_));
}

TcpClient::~TcpClient() {
    INFO("TcpClient::~TcpClient[{}] - connector {}", name_, fmt::ptr(connector_));

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
        // 重新设置TcpConnection中的closeCallback_(有重连功能 -- 这里已经析构不需要重连)为detail::removeConnection
    
        CloseCallback cb = std::bind(&detail::removeConnection, loop_, 
                                        std::placeholders::_1);
        loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
        if (unique) {
            conn->forceClose();
        }
    } else {
        // conn处于未连接状态，酱connetor_停止
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

// 用于连接已经建立的情况下，关闭连接
void TcpClient::disconnect() {
    connect_ = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (connection_) {
            connection_->shutdown();
        }
    }
}

// 可能尚未连接成功，我们停止发起连接
void TcpClient::stop() {
    connect_ = false;
    connector_->stop();
}

TcpConnectionPtr TcpClient::getConnection() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return connection_;
}

void TcpClient::newConnection(int sockfd) {
    loop_->assertInLoopThread();
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    std::string connName = name_ + ":" + peerAddr.getIpPortStr() + "#" + std::to_string(nextConnId_);
    ++nextConnId_;
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(loop_,
                    connName, sockfd, localAddr, peerAddr);
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, std::placeholders::_1)); // FIXME: unsafe
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
        // 这里的重连是指连接断开之后的重连
        connector_->restart();
    }
}





