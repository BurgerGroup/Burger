<div align="center">

<img src="./docs/assets/logo.jpeg" width = "100" height = "80" alt="burger" align=center />

c++11基于协程和reator的高性能Linux服务器框架

![](https://img.shields.io/badge/release-v1.0-blue.svg)
![](https://img.shields.io/badge/build-passing-green.svg)
![](https://img.shields.io/badge/dependencies-up%20to%20date-green.svg)
![](https://img.shields.io/badge/license-MIT-blue.svg)

</div>


-----

## ✨ 特征

- 能使用协程和reactor两种模式构建网络应用
- 现代版的C++11接口，利用c++11新的特性,利用chrono时间，thread库，非阻塞异步接口利用C++11的functional/bind形式的回调仿函数 
- 协程模式中能够用同步的方式表现出异步的特性，简化编码的逻辑
- 封装了spdlog库，更加方便高效地使用高性能日志
- 封装了mysql c api,使得mysql使用更加简单


## ⌛️ 构建

```
$ sudo apt install g++ cmake make libboost-all-dev mysql-server libmysqlclient-dev libcurl4-openssl-dev libbenchmark-dev
 
$ git clone https://github.com/BurgerGroup/Burger.git

$ cd Burger

$ mkdir build && cmake ..

$ make 

$ make install   
```

## 🥇 性能测试

* 阅读 [benchmarks](./benchmarks/README.md)

## 🍭 示例

### 协程echo server:

```cpp
#include <burger/net/CoTcpServer.h>
#include <burger/base/Log.h>
#include <burger/net/RingBuffer.h>

using namespace burger;
using namespace burger::net;

void connHandler(CoTcpConnection::ptr conn) {
    RingBuffer::ptr buffer = std::make_shared<RingBuffer>();
    while(conn->recv(buffer) > 0) {
        conn->send(buffer);
    }
}

int main() {
    LOGGER(); LOG_LEVEL_DEBUG;
    Scheduler sched;
    InetAddress listenAddr(8888);

    CoTcpServer server(&sched, listenAddr);
    server.setConnectionHandler(connHandler);
    server.start();

    sched.wait();
    return 0;
}
```

## 💎 模块

### 配置模块

我们采用ini作为配置文件

定义一个协程的栈大小

```
[coroutine]
stackSize = 3 * 1024 * 1024
```
利用Config::Instance去获取ini配置文件

```
auto& configManager = Config::Instance("/NewConfig/conf.ini");

configManager.getString("coroutine", "stackSize");
````

### 协程模块

协程：用户态的线程，更轻量级。通过hook系统函数，把复杂的异步调用，封装成同步操作。降低业务逻辑的编写复杂度。

采用boost.context里面的fcontext_t的方式实现

```
Coroutine::ptr co = std::make_shared<Coroutine>(func);
```

实际上我们没有向用户暴露co这个概念，用户是不应该自己去创建co的，而是以callback的形式告诉希望完成什么任务。

### 协程调度模块

schedule 负责整个系统的协程调度，Schduler带有一个线程池，而协程的运行依赖于执行器 Processor,每一个线程拥有一个Processor，可以算作一个per thread one processor模型，是一个1：N的协程调度模型。

### Hook模块

hook系统底层和socket相关的API，socket io相关的API，以及sleep系列的API。hook的开启控制是线程粒度的。可以自由选择。通过hook模块，可以使一些不具异步功能的API，展现出异步的性能。

## 📚 文档

* Read 
* benchmarks:
  * [benchmarks性能对比测试](./benchmarks/README.md)
* 组件:
  * [配置文件系统](./docs/configSys.md)
  * [Buffer实现](./docs/buffer.md)
  * [日志系统](./docs/logger.md)
* Coroutine:
  * [协程框架介绍](./docs/co/co_arch.md)
  * [协程reactor对比](./docs/co/co_reactor.md)
  * [协程杂记](./docs/co/co_notes.md)
  * [Hook](./docs/co/hook.md)
  * [FAQ](./docs/co/co_faq.md)
  * [协程TODO](./docs/co/co_todo.md)
* Reactor:
  * [reactor架构](./docs/reactor.md)
  * [tcpServer](./docs/rpc/readme.md)
* RPC
  * [RPC 介绍](./docs/rpc/intro.md)
  * [RPC框架自上向下剖析](./docs/rpc/usage.md)
* example解析
  * [文件传输](./examples/filetransfer/readme.md)
  * [聊天广播](./examples/chat/chat.md)
  * [round trip](./examples/chat/chat.md)
  * [最大连接数](./examples/maxconnection)
  * [timing wheel](./examples/timingWheel/readme.md)
* 工具
  * [性能分析工具](./docs/profiling.md)
  
## 基于Burger的项目

- [BurgerChat](https://github.com/chanchann/BurgerChat) - 🍔 Console-based chat IM for Linux

## Maintainers

[@chanchann](https://github.com/chanchann).

[@skyu98](https://github.com/skyu98).

如有任何问题，请发送邮件 syi78@gatech.edu 交流学习

讨论学习群 :  873966642

## 致谢

感谢[spdlog], [gtest] 等项目, Burger的reactor部分架构深度参考了muduo项目的实现和设计，为上层项目开发而保留，非常感谢Chen Shuo大佬!!!

