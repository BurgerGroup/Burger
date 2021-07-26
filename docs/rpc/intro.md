## RPC

## 集群和分布式

### 单机服务器的局限性

1. 受限于硬件资源，服务器所能承受的用户的并发量有限

2. 任意模块的修改，都会导致整个项目的重新编译(n+ h)，部署(运维复杂)，非常麻烦

3. 系统中，有些模块属于CPU密集型(部署到CPU好的机器上)，有些模块是属于IO密集型, 造成各模块对于硬件资源的需求不一样

### 集群

每一台服务器独立运行一个工程的所有模块。

优点: 用户的并发量提升了（水平拓展），简单

缺点: 项目代码还是需要整体重新编译，而且需要多次部署

有些模块并不需要高并发

### 分布式

一个工程拆分了很多模块 每一个模块独立部署运行在一个服务器主机上

每个节点可做集群。

## 软件模块

1. 大系统的软件模块怎么分

各模块可能会实现大量重复的代码

2. 各模块之间应该怎么访问

各个模块都运行在不同的进程里面 or docker 虚拟环境中

机器1上的模块怎么调用机器2上的模块的一个业务方法呢(涉及网络传输)

我们的分布式框架就是要去隐藏这些细节，像是在本机一样调用函数一样

## 认识RPC框架

RPC : Remote Procedure Call 远程过程调用

比如，我们的登录和聊天功能部署在不同的机器上，那么要完成上面的逻辑，需要依靠网络传输，将要调用的函数名以及参数通过网络序列化后打包发送给另一个机器，然后另一台机器调用后将结果发过来。

## RPC 通信原理 (分布式通信)

```
// RPC 框架流程

      // server1                                   // server2
local call ---> 序列化  --> RPC通信(Burger) -->     反序列化   --> call 
                                                                 |  
local ret  <-- 反序列化<--  RPC通信(Burger) <--      序列化    <-- return

```

我们其中序列化/反序列化 : protobuf 

网络部分，包括寻找rpc服务主机，发起rpc调用请求和相应rpc调用结果，使用Burger网络库和zookeeper 服务配置中心

## protobuf

### Link

https://colobu.com/2015/01/07/Protobuf-language-guide/

### 相对于json好处

1. 二进制存储，xml(20倍)和json(10倍)是文本存储

2. protobuf不需要存储额外的信息

json存储 ： name: "MITSK", pwd : "123"

"zhang san""123456"

- protobuf没有提供任何rpc功能，只是做序列化和反序列化

## protobuf 作用

protobuf 主要是作为 整个框架的传输协议， 我们可以看一下整个框架对于传输信息的格式定义

```
message RpcHeader {
    bytes serviceName = 1;  // 类名
    bytes methodName = 2;   // 方法名
    uint32 argsSize = 3;    // 参数大小
}
```

我们可以看出，它定义了要调用方法是哪一个类，哪一个方法以及方法所需的参数大小

### 框架内传输的数据定义 

4字节标识头部长度 + RpcHeader + args

18UserServiceLogin15MITSK ligh12345

18表示整个类名 + 方法名 + 参数长度的大小 (proto中的定义)

从这个长度我们可以指导，从这个字符流中截取UserServiceLogin15 这18个字符，再根据RpcHeader来反序列化得出类名，方法名，以及参数长度三个重要数据

15表示后面的参数长度, 我们就可以截取参数的字节流 MITSK ligh12345

## protobuf rpc 流程剖析

```cpp
// Closure 是个抽象类
class LIBPROTOBUF_EXPORT Closure {
 public:
  Closure() {}
  virtual ~Closure();

  virtual void Run() = 0;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(Closure);
};
```

## rpc发布服务

```cpp
// Service* 是个基类指针

// RpcServer的使用者，rpc服务方法的发布方

class UserService : public UserServiceRpc {
      login() <== 本地方法
      
      login(controller, request, response, done) <== 重写protobuf提供的virtual虚函数
      {
            // 网络先接收
            1. 从LoginRequest 获取参数的值
            2. 执行本地服务login, 并获取返回值
            3. 用上面的返回值填写LoginResponse
            4. 一个回调，把LoginResponse发送给rpc client
            // 交给网络发送
      }
}

// 问题 : 接收一个rpc调用请求时，它怎么知道要调用应用程序的哪个服务对象的哪个rpc方法呢？
// 比如UserService的Login方法
// 所以我们NotifyService 需要生成一张表，记录服务对象和其发布的所有服务方法
// UserService Login Register 
// FreindService AddFriend DelFriend GetFriendList
// 这些都继承Service类(描述对象)
// Method类(描述方法)
```

```cpp
// 网络(收发)功能有Burger库实现
// protobuf 实现数据的序列化和反序列化
RpcServer provider;
provider.NotifyService(new UserService());
provider.Run();
```


```cpp
burgerRpc::UserServiceRpc_Stub stub(&rpcChannel);  

stub.Login(nullptr, &request, &response, nullptr); 
    
--> 其实是 rpcChannel->callMethod(xxx);


```

```cpp
class RpcChannel : public ::google::protobuf::RpcChannel {
public:
    // 重写此方法
    void CallMethod(xxxx) override;
    // rpc请求的数组组装，数据的序列化
    // 发送rpc请求，wait
    // 接收rpc响应
    // 响应的反序列化
};


```

## 总结下如何发布一个服务

先写proto

protoc生成了pb之后

继承相应的类，然后重写方法，写业务函数

## controller

rpc服务的调用方

rpc 转到 channel的callMethod

如果中间出错直接return，网络没办法返回response

从名字上看出，可以存储一些控制信息，知道当前是什么状态

是个抽象类，里面都是纯虚函数

需要继承重写一下这些方法

## 为什么需要zookeeper

我们并不知道哪个rpc在哪个机器，所以我们需要一个服务配置中心，当启动rpc结点，我们向zk注册一下，

提供分布式协调服务

https://www.cnblogs.com/xinyonghu/p/11031729.html

## zk的数据是怎么组织的 - znode 点

zookeeper数据结构回顾

node_1 path :/node_1

node_1_1 path:/node_1/node_1_1


## 启动zookeeper的结点

conf目录

mv zoo_sample.cfg zoo.cfg

/Burger/thirdparty/zookeeper/conf/zoo.cfg

修改dataDir路径 作为一个存储路径，还是写在磁盘上

clientPort 也可改改

然后进入bin目录

/Burger/thirdparty/zookeeper/bin

启动server脚本

./zkServer.sh


ubuntu直接apt install 

```
sudo apt-get update

sudo apt-get install openjdk-8-jdk

java -version
```

然后ps -ef | grep zookeeper 查看是否启动

netstat -tanp 查看端口

## 如果无法启动

 ./zkServer.sh start-foreground

 ./zkServer.sh: line 170: exec: java: not found

 说明java没装好

 
 ## 启动zk 客户端

 ./zkCli.sh

 常见命令

```
// 连接上
Socket connection established to localhost/127.0.0.1:2181 
```

ls / 看结点

get /zookeeper 查询结点数据

(todo : 了解下字段)

create /MITSK 20 创建结点 + 其数据

set 

delete 

## 