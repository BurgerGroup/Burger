#include "RpcProvider.h"

using namespace burger;
using namespace burger::rpc;

// 这是框架提供给外部使用，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service) {
    
}

// 启动rpc服务结点，开始提供rpc远程网络调用服务
void RpcProvider::Run() {
    
}
