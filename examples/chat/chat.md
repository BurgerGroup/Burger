## 聊天广播服务器 

主要了解如何分包

聊天服务的特点，连接之间的数据有交流，从a连接到的数据要发给b连接

## 考虑的问题

1. 如何一个程序处理多个连接

2. 如何防止串话

3. b可能随时断开连接，新建立的连接c可能恰好复用b的文件描述符，那么a会不会错误的把消息发个c

4. 遇到恶意客户端， 0x00, 0x00, 0x00, 0x09, 'M', 'I','T', 'S', 'K', 我头部写8个，但是其实发了5个，这样就会阻塞，没办法处理了

如何处理错误消息?

- 还会带上一个应用层的校验信息，比如CRC32校验

校验错误，这个消息就是错误消息

- 服务端应该有空闲断开功能

在一定时间没有收到客户端消息，就断开

## codec 

运输层中，TCP是字节流，是无边界的

应用层需要编解码器去分包

## 全局锁带来的开销

server_threaded.cc中，由于mutex的存在，多线程不能并发执行，而是串行的。

```cpp
void onStringMsg(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    for(auto it = connections_.begin(); it != connections_.end(); ++it) {
        codec_.wrapAndSend(*it, msg);
    }
}
```

## 方案 1 : 借助 shared_ptr实现copy on write  -- 降低锁竞争

shared_ptr是引用计数智能指针，如果当前只有一个观察者，那么引用计数为1,可以用shared_ptr::unique()来判断

对于write端，如果发现引用计数为1，这时可以安全地修改对象，不必担心有人在读它。

对于read端，在读之前把引用计数加1，读完之后减1，这样可以保证在读的期间其引用计数大于1，可以阻止并发写。

比较难的是，对于write端，如果发现引用计数大于1，该如何处理?

既然要更新数据，肯定要加锁，如果这时候其他线程正在读，那么不能在原来的数据上修改，得创建一个副本，在副本上修改，修改完了再替换。如果没有用户在读，那么可以直接修改。

核心要点 ： 

1.如果你是数据的唯一拥有者，那么你可以直接修改数据。

2.如果你不是数据的唯一拥有者，那么你拷贝它之后再修改。

```cpp
ConnectionSetPtr getConnSetPtr() {
    std::lock_guard<std::mutex> lock(mutex_);
    return connSetPtr_;
}

void onStringMsg(const std::string& msg) {
    ConnectionSetPtr connSetPtr = getConnSetPtr();
    for(auto it = connSetPtr.begin(); it != connSetPtr.end(); ++it) {
        codec_.wrapAndSend(*it, msg);
    }
}
```

在对象进行读操作，我们getConnSetPtr()将引用计数+1,当离开这个函数，栈上变量离开作用域，引用计数-1

```cpp
void connHandler(const CoTcpConnection::ptr& conn) {
    {
        // 在复本上修改，不会影响读者，所以读者在遍历列表的时候，不需要用mutex保护
        std::lock_guard<std::mutex> lock(mutex_);
        if(!connSetPtr_.unique()) {  // 引用计数大于1
            connSetPtr_.reset(new ConnectionSet(*connSetPtr_)); // 此处是精髓
        }
        assert(connections_.unique());
        connSetPtr_.insert(conn);
    }
```
在进行读操作的时候，当不是唯一的拥有者，我们就需要去拷贝数据然后去更改，

```cpp
connSetPtr_.reset(new ConnectionSet(*connSetPtr_)); // 此处是精髓
```

这里我们new ConnectionSet(*connSetPtr_) 进行复制，这里指向的的是新拷贝的副本，可以进行修改操作，reset，之前的引用计数减1，而读者那边还持有原来那个对象的引用计数继续循环而不用进行mutex保护。

这里我们只需要在连接断开的时候进行加锁，否则我们在onStringMsg()中会一个转发消息给所有客户端之后才进行下一个转发，就无法并发，变成了串行操作

我们此处也只是拷贝了个数据结构，拷贝的损耗也不是很大

但这个办法中，转发hello，达到第一个客户端和最后一个客户端之间延迟仍然比较大

```cpp
void onStringMsg(const std::string& msg) {

    ConnectionSetPtr connSetPtr = getConnSetPtr();
    for(auto it = connSetPtr.begin(); it != connSetPtr.end(); ++it) {
        codec_.wrapAndSend(*it, msg);
    }
}
```

这个转发仍然在单个线程中进行

我们可以分配到多个线程中去执行

## 方案2 : thread_local 实现多线程高效转发

C1 -> S hello T1线程转发消息给所有客户端

C1 -> S hello T2 线程转发消息给C2, T3 线程转发消息给C3

我们定义一个threadLocal connList,使得每个线程拥有不同的connList，这样就不用去加锁保护这个connList，分发任务给多个线程所拥有的conn

```cpp
// 我们在这里让所有的线程addTask -- 分发任务 -- 让各自线程所拥有的conn发送消息
void onStringMsg(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    for(auto it = workProcList_.begin(); it != workProcList_.end(); it++) {
        (*it)->addTask(std::bind(&ChatServer::distributeMsg, this, msg), "distribute msg");
    }
}
```


## todo : loadTest loop quit的问题

为了方便我们benchmark运行多次，我们会在发送完后主动去断开连接

```cpp
// 如果为1s时，如果是burger的服务器会正常， muduo的服务器会~Channel(): Assertion `!addedToEpoll_' failed.
// 这里为什么不同

// 报错的原因容易理解，此处是优雅退出，1s后可能还没有来得及下epoll的二叉树关注从而报错，多等一会
g_loop->runAfter(1.0, std::bind(&EventLoop::quit, g_loop));

g_loop->runAfter(5.0, std::bind(&EventLoop::quit, g_loop));
```