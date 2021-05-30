# Benchmarks
`Burger`的benchmarks总体分为两个部分：
* 1. 各组件的性能测试
    > * Threadpool
    > * Logger
    > * RingBuffer

* 2. 网络库整体的压测(主要在各主流网络库之间进行比较)。
    > * Chat_server简单广播

---
# 组件性能测试
## 1. Threadpool
### 实验环境
实验在**MacBook Pro 13 2019**上进行，使用`Linux Ubuntu 16.04`Docker虚拟机环境.

### 实验对象
* `Muduo::ThreadPool`
* `Boost::threadpool::threadpool<>`
* `Burger::Threadpool`

### 实验内容
使用上述三个线程池，在相同的条件下：
* 1. 相同的实验环境
* 2. 指定$100$个工作线程
* 3. 不设置任务队列最大限制
* 4. 都为Release版本

分别完成$300000$次相同的任务：
```cpp
void task()
{
    std::this_thread::sleep_for(milliseconds(2)); // 模拟处理任务
    ++counter; // std::atomic<std::uint64_t> counter(0);
}
```
在所有任务完成之后，统计总共的执行时间.

### 实验结果
进行了5次实验，结果记录如下(时间单位为ms )：
||Muduo|Boost|Burger|
|--|--|--|--|
|1|6982|6696|6692|
|2|7036|6713|6689|
|3|6958|6699|6687|
|4|7007|6688|6705|
|5|7005|6671|6695|
|**平均总任务时间**|6997.6|6693.4|6693.6|
|**平均任务数/s**|42.872|44.820|44.818|


可以看出，在本次的实验当中，`Boost`表现最好，而`Burger`以微弱的差别紧随其后；`Muduo`用时较长的原因，可能是在于其使用了自己实现的`Mutex`、`Thread`等类。

---

## 2. Logger
### 实验环境
实验在**MacBook Pro 13 2019**上进行，使用`Linux Ubuntu 16.04`Docker虚拟机环境.

### 实验对象
* `Boost::logger`
* `Burger::Logger(Based on spdLog)`
* **TODO:**`Muduo::logger`

### 实验内容
使用上述库中的Logger，在相同的条件下：
* 1. 相同的实验环境
* 2. **同步/异步模式下都只指定一个日志工作线程**
* 3. 缓冲池最大限制相同（8192）
* 4. 同样的输出格式

分别完成下列场景的任务：
* 1. **console/file**
* 2. **同步/异步（async/sync）**
* 3. **多线程/单线程模式(mt/st)**
* 4. **32Byte/512Byte**

使用`Google::benchmark`工具分别迭代$20000$次：
```cpp
// spdlog
auto BM_spdlog = [](benchmark::State& state, bool isAsync, bool isToFile, bool isMultiThread = false)-> void {
    // init logger
    // ...
    string msg(state.range(0), 's');
    for (auto _ : state){
        SPDLOG_LOGGER_INFO(logger, msg);
    }
    
    // compute the total bytes processed
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));

    // compute the total iteraitons processed
};
```
统计各种场景下的平均执行速度.

### 实验结果

* 1. **console/file**

||async_console_st/32|async_file_st/32|async_console_st/512|async_file_st/512|
|--|--|--|--|--|
|spdlog|93.419k/s|71.817k/s|122.173k/s|87.116k/s|
|boost::log|61.110k/s||58.576k/s||


* 2. **异步Async/同步Sync**

||sync_console_st/32|sync_file_st/32|sync_file_mt/32|
|--|--|--|--|
|spdlog|2.600M/s|2.803M/s|3.853M/s|
|boost::log||||


--- 

## 3. Buffer
### 实验环境
实验在**MacBook Pro 13 2019**上进行，使用`Linux Ubuntu 16.04`Docker虚拟机环境.

### 实验对象
* `Burger::net::RingBuffer`
* `Muduo::net::Buffer`

### 实验内容
使用上述两种缓冲区，在相同的条件下：
* 1. 相同的缓冲区大小
* 2. 相同的读写操作

分别完成下列场景的任务（主要是腾挪的数据大小比例）：
* 1. **需要腾挪95%+的数据**
* 2. **需要腾挪50%的数据**
* 3. **需要腾挪10%的数据**


使用`Google::benchmark`工具分别迭代$500000$次读写任务：
```cpp
void rwBuffer(IBuffer& buf, size_t len) {
    buf.append(msg);
    buf.retrieve(len);
    buf.append(std::string(len - 8, 's'));
    std::string tmp(8, 's');
    buf.prepend(reinterpret_cast<const void*>(tmp.c_str()), 8); // 把前面填满
}
```
在所有任务完成之后，统计平均的执行速度.

### 实验结果
||95%+|50%|5%|
|--|--|--|--|
|RingBuffer|33.938G/s|18.186G/s|13.4162G/s|
|Buffer|18.2195G/s|14.542G/s|13.2121G/s|

* 可以看出，在**有大量内部腾挪的场景下**，`RingBuffer`的读写速度有着很明显的优势，**提高了$ 86.5\% $.**

* 随着内部腾挪的数据占比下降，`RingBuffer`的读写速度优势也在下降；在几乎没有内部腾挪时，其读写速度和普通`Buffer`相当（在少数实验中，甚至会略微低一些）。

* 因此，我们在`Burger`中保留了两种`Buffer`供用户选择；用户也可以通过继承`IBuffer基类`实现自己的`Buffer类`


---
# 网络库整体压测
在本部分测试中，使用的是`Burger/examples`中的示例程序。在各文件夹下，使用其中的脚本进行不同压力或不同场景下的测试。
## 1. Chat_server广播
### 实验环境
实验在**MacBook Pro 13 2019**上进行，使用`Linux Ubuntu 16.04`Docker虚拟机环境.

### 实验对象
* Muduo示例程序中的`asio_chat_server`系列
* Burger示例程序中的`chat_server`系列

### 实验内容
这里的`Chat_server`将一条`Message`广播给$K$个`Client`，由`Client`端记录**最先和最后接收到消息的时间**。
我们关注5次实验的**广播消息的平均总用时**。
具体程序见**examples/chat/loadtest.cc**。

### 实验结果
这里我们挑选了两个效率较高的Server版本 **(Efficient/High-Performance)** 来进行压测对比，它们比单纯的多线程Server效率更高。具体请见`example/chat/chat.md`。
实验结果如下：

* $K$:发起连接的客户端个数
* 表格中数据为完成广播消息的平均总时间（ms），数值越小代表性能越好。

|K|Burger::Efficient|Muduo::Efficient|Burger::HighPerformance|Muduo::HighPerformance|
|:--:|:--:|:--:|:--:|:--:|
|10|0.183|0.246|0.135|0.142|
|100|1.744|0.763|1.100|1.111|
|10000|97.631|65.161|53.187|93.783|
|20000|134.402|132.415|97.488|110.903|

* 1. 通过上述数据可以看出，两者的性能较为接近，并且随着连接数的增加，差距逐渐缩小
* 2. 在实验中，发现**有时候实验结果波动较大**，推测是虚拟机的一些原因；所以在多次实验后，在**较为平稳的一段时间内完成了本次对比；但是也无法忽略其影响**。
* 3. 由此，只能得出**Burger的协程库搭建的Server，在该场景下并不弱于Muduo的基于Reactor模型的Server**。
* 4. 在用$20000$个Client对Muduo的server发起连接时，有时候server会出现无响应的现象；还需进一步研究该现象的原因。


