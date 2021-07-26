#include <iostream>
#include <string>
#include "user.pb.h"
#include "burger/rpc/RpcServer.h"
#include "burger/base/Log.h"

using namespace burger; 
using namespace burger::rpc;

// 使用在rpc服务发布端(rpc服务发布者)
class UserService : public burgerRpc::UserServiceRpc {  
public:
    bool Login(std::string& name, std::string& pwd) {
        std::cout << "Doing local service : Login" << std::endl; 
        std::cout << "name: " << name << " pwd: " << pwd << std::endl;
        return true;
    }

    bool Register(uint32_t id, std::string name, std::string pwd) {
        std::cout << "Doing local service : Register" << std::endl; 
        std::cout << "id: " << id << " name: " << name << " pwd: " << pwd << std::endl;
        return true;
    }

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

    void Register(::google::protobuf::RpcController* controller,
                       const ::burgerRpc::RegisterRequest* request,
                       ::burgerRpc::RegisterResponse* response,
                       ::google::protobuf::Closure* done) {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool ret = Register(id, name, pwd);
        
        response->mutable_result()->set_errcode(0);
        response->set_success(ret);

        done->Run();
    }
};

int main(int argc, char **argv) {
    LOGGER(); LOG_LEVEL_INFO;
    // provider是一个rpc网络服务对象，把UserService对象发布到rpc结点上
    RpcServer provider;
    UserService userService;
    provider.NotifyService(&userService);
    // 可以多次
    // provider.NotifyService(new ProductService);
    // 启动一个rpc服务发布节点, Run()以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

    return 0;
}