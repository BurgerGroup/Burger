#include "RpcProvider.h"

using namespace burger;
using namespace burger::net;
using namespace burger::rpc;
using namespace std::placeholders;

// 这是框架提供给外部使用，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service) {
    
}

// 启动rpc服务结点，开始提供rpc远程网络调用服务
void RpcProvider::Run() {
    std::string ip = Config::Instance().getString("rpc", "rpcServerIp", "127.0.0.1");
    uint16_t port = Config::Instance().getUInt16("rpc", "rpcServerPort", 8000);
    InetAddress addr(ip, port);
    CoTcpServer server(&sched_, addr, "RpcProvider");

    server.setConnectionHandler(std::bind(&RpcProvider::connHandler, this, _1));
    server.setThreadNum(4);
    INFO("RpcProvider start service at {} : {}", ip, port);
    server.start();
    sched_.wait();
}

void RpcProvider::connHandler(const CoTcpConnection::ptr& conn) {
    
}
