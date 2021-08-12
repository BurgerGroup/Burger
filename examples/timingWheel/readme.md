## timing wheel 踢掉空闲连接

一个连接若干秒没有收到消息，认定是空闲连接

一般在应用层中用心跳协议判断对方是否正常工作

此处timing wheel权宜之计，学学shared_ptr和weak_ptr用法

## 原始暴力方法 

1. 注册一个每过1s，重复执行的定时器

```cpp
Scheduler::runEvery(1s, func);

// 每隔一秒, 遍历整个连接列表（费时），判断是否超时，如果超时，就断开
foreach(conn in connList) {
    if(now - conn.lastReceiveTime > 8s) {
        conn->shutdown();
    }
}

// 每次消息到来，更新lastReceiveTime
```

2. 为每个连接注册一个一次性的定时器，超过时间8s，断开，收到消息重新更新定时器8s

但是可能连接较多，这样对timerQueue压力较大

## timing wheel
* 环形队列(只用到tail指针)，有8个`bucket`，使用`circular-buffer`保存

* 当创建连接/有数据到来时，将本连接放至队尾的`bucket`

* 事件循环中添加**定时器**，每$1$s会弹出队首的`bucket`并析构之；显然若其中的连接引用计数递减后为0，则该连接被析构断开。