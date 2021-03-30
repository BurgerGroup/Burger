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