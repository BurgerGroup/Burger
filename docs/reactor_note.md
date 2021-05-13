## 

TODO: 我们的协程架构设计的有问题

## 三种常用模型

reactor : 一个IO线程

multiple reacotr : 多个IO线程

one loop per thread + thread pool (多个IO线程 + 计算线程池)

## multiple reacotr

main reactor : 负责listenfd accept

sub reactor : 负责connfd

round robin 轮叫

来一个连接就选择下一个EventLoop

这样就让多个连接分配给若干个EventLoop来处理，而每个EventLoop属于一个IO线程

也就意味着，多个连接分配给若干个IO线程来处理

## one loop per thread + thread pool

sudoku求解服务器

既是IO密集型，又是计算密集型的一个服务

IO 线程池 + 计算线程池

计算时间如果比较久，就会使得IO线程阻塞，就不能处理大量的并发连接

要把计算部分放到计算线程池里

见sudoku/server_threadpool.cc

一个IO线程+计算机线程池

TODO: 还要考虑把工作线程写一个set接口去生成，而不是用构造函数，参考下muduo实现

todo: 考虑send跨线程调用

计算线程池中计算完后，再通过IO线程池发送

## 再次总结

reactor关注三个半事件

- 连接建立
- 连接断开
- 消息到达
- 消息发送完毕(低流量服务来说，通常不需要关注这个)
