## RPC

## 集群和分布式

### 单机服务器的局限性

1. 受限于硬件资源，服务器所能承受的用户的并发量有限

2. 任意模块的修改，都会导致整个项目的重新编译(n+ h)，部署(运维复杂)，非常麻烦

3. 系统中，有些模块属于CPU密集型(部署到CPU好的机器上)，有些模块是属于IO密集型, 造成各模块对于硬件资源的需求不一样

### 集群

每一台服务器独立运行一个工程的所有模块。

优点: 用户的并发量提升了（水平拓展）， 简单

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

## RPC 通信原理 (分布式通信)

```
      // server1                                   // server2
local call ---> 序列化  --> RPC通信(Burger) -->     反序列化   --> call 
                                                                 |  
local ret  <-- 反序列化<--  RPC通信(Burger) <--      序列化    <-- return


// 序列化/反序列化 : protobuf 
// 网络部分，包括寻找rpc服务主机，发起rpc调用请求和相应rpc调用结果，使用Burger网络库和zookeeper 服务配置中心
```

## protobuf

相对于json好处

1. 二进制存储，xml(20倍)和json(10倍)是文本存储

2. protobuf不需要存储额外的信息

json存储 ： name: "MITSK", pwd : "123"

"zhang san""123456"

- protobuf没有提供任何rpc功能，只是做序列化和反序列化

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


