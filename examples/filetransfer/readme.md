## 用发送文件说明CoTcpConnection::send()的使用

send 返回值为void,用户不必关心调用send()时发送多少字节，会保证把数据发送给对方

send 非阻塞，即便TCP发送窗口满了，也绝不会阻塞当前调用的线程

send 线程安全  -- 注意原子性

## 探究 原子性

reactor模式的send具有原子性

多个线程同时调用，消息之间不会混叠或交织

假设两个线程同各自发送上一条任意长度的消息，要么先a后b，or 先b后a，不会出现a的前一半，b，a的后一半

在rector模式中，原子性是由outbuffer来保证的，发送a若未发送完全，那么加入outbuffer中，若b也未发送完，那么也加入outbuffer,而在outbuffer中数据的先后是有序的

而协程模式虽然我们的send无原子性，如果对应多个task，我第一个task send导致了发送收缓冲区满，我切换出去epoll关注，执行第二个task，这样导致了我们消息的交织，但是这种场景本来就不符合我们同步写法，我们一个conn对应一个task，就是我们的connHandler，我们的逻辑中有一个send，这个send执行完毕，我们才进行下一轮的数独求解。

## 三种文件传输模式

1. 一次性读入内存

2. 一块一块发送

3. 同2，shared_ptr管理FILE*


