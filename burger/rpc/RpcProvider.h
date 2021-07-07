#ifndef RPCPROVIDER_H
#define RPCPROVIDER_H
#include <google/protobuf/service.h>
#include <memory>
#include "burger/net/TcpServer.h"
#include "burger/net/Scheduler.h"
#include "burger/net/InetAddress.h"

namespace burger {
namespace rpc {

// 框架提供的专门服务发布rpc服务端网络对象类
class RpcProvider {
public:
    // 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);
    
    // 启动rpc服务接点，开始提供rpc远程网络调用服务
    void Run();
private:
    std::unique_ptr<burger::net::TcpServer> tcpServerPtr_;
    burger::net::Scheduler *sched_;
};

} // namespace rpc

} // namespace burger




#endif // RPCPROVIDER_H