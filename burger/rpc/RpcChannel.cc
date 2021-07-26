#include "RpcChannel.h"
#include "burger/rpc/rpcHeader.pb.h"
#include "burger/rpc/RpcController.h"
#include "burger/base/Log.h"
#include "burger/base/Config.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace burger;
using namespace burger::rpc;

// todo: 这个函数太长了，需要拆分一下，使得整个逻辑更清晰
// header_size + service_name method_name args_size + args
void RpcChannel::CallMethod(const ::google::protobuf::MethodDescriptor* method,
                ::google::protobuf::RpcController* controller,
                const ::google::protobuf::Message* request,
                ::google::protobuf::Message* response,
                ::google::protobuf::Closure* done) {
    
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string serviceName = sd->name();   
    std::string methodName = method->name();

    std::string argsStr;
    size_t argsSize = 0;
    if(request->SerializeToString(&argsStr)) {
        argsSize = argsStr.size();
    } else {
        controller->SetFailed("Serialize request error");
        return;
    }
    RpcHeader rpcHeader;
    rpcHeader.set_servicename(serviceName);
    rpcHeader.set_methodname(methodName);
    rpcHeader.set_argssize(static_cast<google::protobuf::uint32>(argsSize));

    std::string rpcHeaderStr;
    size_t headerSize = 0;
    if(rpcHeader.SerializeToString(&rpcHeaderStr)) {
        headerSize = rpcHeaderStr.size();
    } else {
        controller->SetFailed("Serialize RPC header error!");
        return;
    }
    // 组织待发送的rpc请求的字符串
    std::string sendRpcStr;
    sendRpcStr.insert(0, std::string(reinterpret_cast<char *>(&headerSize), 4));
    sendRpcStr += rpcHeaderStr;
    sendRpcStr += argsStr;

#ifdef DEBUG
        INFO("====================================");
        INFO("headerSize : {}", headerSize);
        INFO("rpcHeaderStr : {}", rpcHeaderStr);
        INFO("serviceName : {}", serviceName);
        INFO("methodName : {}", methodName);
        INFO("argsSize : {}", argsSize);
        INFO("argsStr : {}", argsStr);
        INFO("====================================");
#endif

    // 使用tcp编程，完成rpc方法的远程调用, 此处不需要高并发
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd) {
        // sprintf()最常见的应用之一莫过于把整数打印到字符串中
        char errtxt[64];
        sprintf(errtxt, "create socket error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    std::string ip = Config::Instance().getString("rpc", "rpcServerIp", "127.0.0.1");
    uint16_t port = Config::Instance().getUInt16("rpc", "rpcServerPort", 8000);
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 连接rpc服务节点
    if (-1 == connect(clientfd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr))) {
        close(clientfd);
        char errtxt[64];
        sprintf(errtxt, "send error, errno = %d", errno);
        controller->SetFailed(errtxt);
        return;
    }
    
    // 发送rpc请求
    if (-1 == send(clientfd, sendRpcStr.c_str(), sendRpcStr.size(), 0)) {
        close(clientfd);
        char errtxt[64];
        sprintf(errtxt, "send error! errno = %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 接收rpc请求的响应值
    char recv_buf[1024] = {0};
    ssize_t recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0))) {
        close(clientfd);
        char errtxt[64];
        sprintf(errtxt, "recv error ! errno = %d", errno);
        controller->SetFailed(errtxt);
        return;
    }
    
    // 反序列化rpc调用的响应数据

    // bug 出现问题， rev_buf中遇到\0后面的数据就存不下来了
    // gdb : p recv_buf vs p responseStr 对比下， string 构造函数里的坑
    // std::string responseStr(recv_buf, 0, recv_size);   
    // if(!response->ParseFromString(responseStr)) {
    if(!response->ParseFromArray(recv_buf, static_cast<int>(recv_size))) {
        close(clientfd);
        char errtxt[2048];
        sprintf(errtxt, "recv error ! errno = %s", recv_buf);
        controller->SetFailed(errtxt);
        return;
    }
    close(clientfd);
}

