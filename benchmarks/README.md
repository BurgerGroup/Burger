# Benchmarks
`Burger`的benchmarks总体分为两个部分：
* 1. 各组件的性能测试
    > * Threadpool
    > * Logger
* 2. 网络库整体的压测
    > 主要在各主流网络库之间进行比较。

---
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
