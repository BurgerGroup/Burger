## 框架的自上向下剖析

代码在/Burger/burger/rpc/example文件夹下

## 如何发布一个Service(启动一个RpcServer)

先写proto，定义好如何收发(service)的格式(是什么函数名，参数是什么字段，返回什么)

我们需要定义proto的message类型，进行数据的序列化和反序列化

```
syntax = "proto3";

package burgerRpc;

option cc_generic_services = true;

message ResultCode {
    int32 errcode = 1;
    bytes errmsg = 2;
}
message LoginRequest {
    bytes name = 1;
    bytes pwd = 2;
}

message LoginResponse {
    ResultCode result = 1;
    bool success = 2;
}

message RegisterRequest {
    uint32 id = 1;
    bytes name = 2;
    bytes pwd = 3;
}

message RegisterResponse {
    ResultCode result = 1;
    bool success = 2;
}

// 最重要的就是 函数名 函数参数 返回 三个部分
service UserServiceRpc {
    // 不一定和函数名相等，但是最好对应起来
    rpc Login(LoginRequest) returns(LoginResponse);
    rpc Register(RegisterRequest) returns(RegisterResponse);
}

```

我们写一个UserService去继承我们生成user.pb.h中的UserServiceRpc

UserService 原来是一个本地服务，提供了两个进程内的本地方法，Login 和 Register, (我们只拿其中的Login举例)

```cpp
// 使用在rpc服务发布端(rpc服务发布者)
class UserService : public burgerRpc::UserServiceRpc {  
public:
    // 进程内的本地方法
    bool Login(std::string& name, std::string& pwd) {
        std::cout << "Doing local service : Login" << std::endl; 
        std::cout << "name: " << name << " pwd: " << pwd << std::endl;
        return true;
    }
    .......
};
```

然后重写基类UserService 的虚函数

我们发布者(就是一个RpcServer)，有个request来请求，以下这个方法就是当有请求来的时候框架帮我们调用的 

```cpp
void Login(::google::protobuf::RpcController* controller,
                    const ::burgerRpc::LoginRequest* request,
                    ::burgerRpc::LoginResponse* response,
                    ::google::protobuf::Closure* done) {
    // 反序列化，应用获取相应数据做本地业务
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
```

发布

```cpp
// provider是一个rpc网络服务对象，把UserService对象发布到rpc结点上
RpcServer provider;
UserService userService;
provider.NotifyService(&userService);
// 可以多次 provider.NotifyService(&userService);
// 启动一个rpc服务发布节点, Run()以后，进程进入阻塞状态，等待远程的rpc调用请求
provider.Run();
```

这样我们就发布了UserService

## RpcServer 

我们这里发布Service是靠 RpcServer

```cpp
// 框架提供的专门服务发布rpc服务端网络对象类
class RpcServer {
public:
    // 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);
    void Run();  // 启动rpc服务接点，开始提供rpc远程网络调用服务
private: 
    void connHandler(const net::CoTcpConnection::ptr& conn);
    void sendRpcResonse(const net::CoTcpConnection::ptr& conn, google::protobuf::Message*);
private:
    net::Scheduler sched_;
    struct ServiceInfo {
        google::protobuf::Service *service_;   // 保存服务对象
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> methodMap_;
    }; 
    std::unordered_map<std::string, ServiceInfo> serviceInfoMap_;
};
```

### 函数剖析

```cpp
void RpcServer::NotifyService(google::protobuf::Service *service) {
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
```

大致流程是

1. 先获取service

2. 然后获取service里面的method

3. 保存下service中的method 映射

4. 保存下service映射

```cpp
// 启动rpc服务结点，开始提供rpc远程网络调用服务
void RpcServer::Run() {
    std::string ip = Config::Instance().getString("rpc", "rpcServerIp", "127.0.0.1");
    uint16_t port = Config::Instance().getUInt16("rpc", "rpcServerPort", 8000);
    InetAddress addr(ip, port);
    CoTcpServer server(&sched_, addr, "RpcServer");

    server.setConnectionHandler(std::bind(&RpcServer::connHandler, this, _1));
    server.setThreadNum(4);
    INFO("RpcServer start service at {} : {}", ip, port);
    server.start();
    sched_.wait();
}
```

Run就是启动服务器

```cpp
void RpcServer::connHandler(const CoTcpConnection::ptr& conn) {
    Buffer::ptr buf = std::make_shared<Buffer>();
    while(conn->recv(buf) > 0) {
        // 网络上接受的远程rpc调用请求的字符流， Login args
        std::string recvStr = buf->retrieveAllAsString();
        uint32_t headerSize = 0;
        std::string rpcHeaderStr = readHeader(recvStr, headerSize);
        
        std::string serviceName;
        std::string methodName;
        uint32_t argsSize = 0;
        if(!deserializeHeader(rpcHeaderStr, serviceName, methodName, argsSize)) return;
        
        std::string argsStr = readArgs(recvStr, headerSize, argsSize);
#ifdef DEBUG
        DEBUG("headerSize : {}", headerSize);
        DEBUG("rpcHeaderStr : {}", rpcHeaderStr);
        DEBUG("serviceName : {}", serviceName);
        DEBUG("methodName : {}", methodName);
        DEBUG("argsSize : {}", argsSize);
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
        // todo: 为何我们此处要手动去设置类型
        google::protobuf::Closure *done = google::protobuf::NewCallback<RpcServer,
                                                            const CoTcpConnection::ptr&,
                                                            google::protobuf::Message *>
                                                            (this, 
                                                            &RpcServer::sendRpcResonse, 
                                                            conn, response);
        // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
        // new UserService().Login(controller, request, response, done)
        service->CallMethod(method, nullptr, request, response, done);
    }
}
```

最为核心的就是这个connHandler 网络处理逻辑。

在框架内部，RpcServer 和 RpcConsumer 协商好之间通信的protobuf数据类型

e.g 发来字节流 : 16UserserviceMITSK123

headerSize(4个字节) + headerStr + argsStr

考虑粘包问题，所以我们不仅要记录serviceName, methodName 还要记录 argsSize

## 大致流程

```
1. Caller --> Login(LoginRequest) 序列化 --> Burger -->  callee 
2. calee --> Login(LoginRequest)  --> 交到下面重写的Login(controller, request,response, done) 方法上
```