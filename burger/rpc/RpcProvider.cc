#include "RpcProvider.h"
#include "rpcHeader.pb.h"
#include <google/protobuf/descriptor.h>
#include "burger/net/Buffer.h"

using namespace burger;
using namespace burger::net;
using namespace burger::rpc;
using namespace std::placeholders;

// 这是框架提供给外部使用，可以发布rpc方法的函数接口
// service_name --》 service描述  --》 service* 记录的服务对象
// --》 method_name --> method 方法对象
void RpcProvider::NotifyService(google::protobuf::Service *service) {
    ServiceInfo serviceInfo;
    // 获取了服务对象的描述信息
    const google::protobuf::ServiceDescriptor *serviceDescPtr = service->GetDescriptor();
    // 获取服务的名字
    const std::string serviceName = serviceDescPtr->name();
    // 获取服务对象的方法的数量
    int methodCnt = serviceDescPtr->method_count();
    INFO("Service Name : {}", serviceName);
    for(int i = 0; i < methodCnt; i++) {
        // 获取了服务对象指定下标的服务方法的描述(抽象描述)
        const google::protobuf::MethodDescriptor *methodDescPtr = serviceDescPtr->method(i);
        std::string methodName = methodDescPtr->name();
        INFO("Method Name : {}", methodName);
        serviceInfo.methodMap_.insert({methodName, methodDescPtr});
    }
    serviceInfo.service_ = service;
    serviceInfoMap_.insert({serviceName, serviceInfo});
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

// 框架内部，RpcProvider 和 RpcConsumer 协商好之间通信的protobuf数据类型
// service_name method_name
// 我们需要定义proto的message类型，进行数据头的序列化和反序列化
// 发来字节流 : 16UserserviceMITSK123
// header_size(4个字节) + header_str + args_str
// 考虑粘包问题，所以我们不仅要记录service_name, method_name 还要记录 args_size
// header_size 要存成二进制， 利用std::string 的copy
// todo : 此处好好反复考虑粘包情况
void RpcProvider::connHandler(const CoTcpConnection::ptr& conn) {
    Buffer::ptr buf = std::make_shared<Buffer>();
    while(conn->recv(buf) > 0) {
        // 网络上接受的远程rpc调用请求的字符流， Login args
        std::string recvStr = buf->retrieveAllAsString();
        // 从字节流中读取4个字节的内容
        uint32_t headerSize = 0;
        size_t headerPrefixNum = 4; 
        recvStr.copy(reinterpret_cast<char *>(&headerSize), headerPrefixNum, 0);

        // 根据headerSize读取数据头的原始字符流
        std::string rpcHeaderStr = recvStr.substr(headerPrefixNum, headerSize); 
        // 反序列化数据，得到rpc请求的详细信息
        RpcHeader rpcHeader;
        std::string serviceName;
        std::string methodName;
        uint32_t argsSize = 0;
        if(rpcHeader.ParseFromString(rpcHeaderStr)) {
            // 数据头反序列化成功
            serviceName = rpcHeader.servicename();
            methodName = rpcHeader.methodname();
            argsSize = rpcHeader.argssize();
        } else {
            // 数据头反序列化失败
            ERROR("rpcHeaderStr {} parse error", rpcHeaderStr);
            return;
        }
        // 获取rpc方法参数的字符流数据
        std::string argsStr = recvStr.substr(headerPrefixNum + headerSize, argsSize);
#ifdef DEBUG
        INFO("====================================");
        INFO("headerSize : {}", headerSize);
        INFO("rpcHeaderStr : {}", rpcHeaderStr);
        INFO("serviceName : {}", serviceName);
        INFO("methodName : {}", methodName);
        INFO("argsSize : {}", argsSize);
        INFO("====================================");
#endif
        // 获取service对象和method对象
        auto it = serviceInfoMap_.find(serviceName);
        if(it == serviceInfoMap_.end()) {
            ERROR("Service Name dose not exist");
            return;
        }

        auto mit = it->second.methodMap_.find(methodName);
        if(mit == it->second.methodMap_.end()) {
            ERROR("method Name : {} dose not exist", methodName);
        }
        google::protobuf::Service *service = it->second.service_;  // 获取service对象 new UserService
        const google::protobuf::MethodDescriptor *method = mit->second;  // 获取method对象 Login

        // 生成rpc方法调用的请求request 和 响应 response 参数
        google::protobuf::Message *request = service->GetRequestPrototype(method).New();
        if(!request->ParseFromString(argsStr)) {
            ERROR("request parse error, content : {}", argsStr);
            return;
        }
        google::protobuf::Message *response = service->GetResponsePrototype(method).New();
        
        // 下面的method方法的调用，绑定一个Closure的回调函数
<<<<<<< HEAD
        google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider,
                                                            const CoTcpConnection::ptr&,
                                                            google::protobuf::Message*>
=======
        // todo: 为何我们此处要手动去设置类型
        google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider,
                                                            const CoTcpConnection::ptr&,
                                                            google::protobuf::Message *>
>>>>>>> 7ffcd9bef8095f65ca9a2e9f76f24a42810abd1e
                                                            (this, 
                                                            &RpcProvider::sendRpcResonse, 
                                                            conn, response);
        // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
        // new UserService().Login(controller, request, response, done)
        service->CallMethod(method, nullptr, request, response, done);
    }
}

// Closure 的回调操作，用于序列化rpc的响应和网络发送
void RpcProvider::sendRpcResonse(const CoTcpConnection::ptr& conn, google::protobuf::Message* response) {
    std::string responseStr;
    if(response->SerializeToString(&responseStr)) {   // response进行序列化
        // 序列化成功后，通过网络把rpc方法执行的结果发送回rpc的调用方
        conn->send(responseStr);
    } else {
        ERROR("Serialize response string error");
    }
    conn->shutdown();   // 短连接，主动关闭
}
