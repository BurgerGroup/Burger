#include <iostream>
#include <string>
#include "../user.pb.h"
#include "burger/rpc/RpcApp.h"
#include "burger/rpc/RpcProvider.h"

using namespace burger; 
using namespace burger::rpc;

// UserService 原来是一个本地服务，提供了两个进程内的本地方法，Login 和GetFriendLists
// 如何发布
class UserService : public burgerRpc::UserServiceRpc {  // 使用在rpc服务发布端(rpc服务发布者)
public:
    bool Login(std::string& name, std::string& pwd) {
        std::cout << "Doing local service : Login" << std::endl; 
        std::cout << "name: " << name << " pwd: " << pwd << std::endl;
        return true;
    }
    /*
    先写proto，定义好如何收发(service)的格式(是什么函数名，参数是什么字段，返回什么)
    1. Caller --> Login(LoginRequest) 序列化 --> Burger -->  callee 
    2. calee --> Login(LoginRequest)  --> 交到下面重写的Login方法上
    */
    // 重写基类UserService 的虚函数, 下面这些方法可都是框架直接调用的
    // 这个Login是框架帮我们调用的
    void Login(::google::protobuf::RpcController* controller,
                       const ::burgerRpc::LoginRequest* request,
                       ::burgerRpc::LoginResponse* response,
                       ::google::protobuf::Closure* done) {
        // 框架给业务上报了请求参数LoginRequest，业务反序列化
        // 应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool login_reult = Login(name, pwd);  // 先用req解析出本地业务需要的参数，然后做本地业务

        // 添加好相应消息
        burgerRpc::ResultCode* code = response->mutable_result();  
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_reult);

        // 执行回调函数  执行响应对象数据的序列化和网络发送(都是由框架来完成的)
        done->Run();
    }
};

int main(int argc, char **argv) {
    // 调用框架的初始化操作
    RpcApp::Init(argc, argv);

    // provider是一个rpc网络服务对象，把UserService对象发布到rpc结点上
    RpcProvider provider;
    provider.NotifyService(new UserService());
    // 可以多次
    // provider.NotifyService(new ProductService);
    // 启动一个rpc服务发布节点, Run()以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

    return 0;
}