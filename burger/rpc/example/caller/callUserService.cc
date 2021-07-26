#include <iostream>
#include "user.pb.h"
#include "burger/rpc/RpcChannel.h"
#include "burger/base/Log.h"
using namespace burger;
using namespace burger::rpc;

int main() {
    // 调用远远程发布的rpc方法Login
    RpcChannel rpcChannel;
    burgerRpc::UserServiceRpc_Stub stub(&rpcChannel);  

    burgerRpc::LoginRequest request;
    request.set_name("MITSK");
    request.set_pwd("123");

    burgerRpc::LoginResponse response;
    // 发起rpc方法的调用， 同步的rpc调用过程 RpcChannel::callMethod
    stub.Login(nullptr, &request, &response, nullptr);  // RpcChannel->RpcChannel::callMethod() 集中来做所有rpc方法调用的参数序列化和网络发送
    
    // 一次rpc调用完成，读调用的结果
    if(0 == response.result().errcode()) {
        INFO("RPC Login response success: {}", response.success());
    } else {
        ERROR("RPC Login response error : {}", response.result().errmsg());
    }

    // 演示调用远程发布的rpc方法Register
    burgerRpc::RegisterRequest req;
    req.set_id(2000);
    req.set_name("MIT SK");
    req.set_pwd("6666");
    burgerRpc::RegisterResponse rsp;

    // 以同步的方式发起rpc调用请求，等待返回结果
    stub.Register(nullptr, &req, &rsp, nullptr);
    if(0 == response.result().errcode()) {
        INFO("RPC Register response success: {}", response.success());
    } else {
        ERROR("RPC Register response error : {}", response.result().errmsg());
    }
    return 0;
}