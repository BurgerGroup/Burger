#ifndef RpcServer_H
#define RpcServer_H

#include <google/protobuf/service.h>
#include <unordered_map>
#include "burger/net/InetAddress.h"
#include "burger/net/CoTcpServer.h"
#include "burger/net/CoTcpConnection.h"
#include "burger/net/Scheduler.h"
#include "burger/base/Config.h"

namespace burger {
namespace rpc {

// 框架提供的专门服务发布rpc服务端网络对象类
class RpcServer {
public:
    // 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
    void NotifyService(::google::protobuf::Service *service);
    void Run();  // 启动rpc服务接点，开始提供rpc远程网络调用服务
private: 
    void connHandler(const net::CoTcpConnection::ptr& conn);
    void sendRpcResonse(const net::CoTcpConnection::ptr& conn, google::protobuf::Message *);

    std::string readHeader(const std::string& recvStr, uint32_t& headerSize);
    bool deserializeHeader(const std::string& rpcHeaderStr, std::string& serviceName, 
                std::string& methodName, uint32_t& argsSize); // 反序列化头部数据，得到rpc请求的详细信息
    std::string readArgs(const std::string& recvStr, const uint32_t headerSize, const uint32_t argsSize);
    
private:
    net::Scheduler sched_;
    struct ServiceInfo {
        google::protobuf::Service *service_;   // 保存服务对象
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> methodMap_;
    }; 
    std::unordered_map<std::string, ServiceInfo> serviceInfoMap_;
};

} // namespace rpc

} // namespace burger




#endif // RpcServer_H