# 遇到的的一些问题和思考

## 1. `Burger::TcpServer`/ `Burger::CoTcpServer`的瓶颈
### 描述:
* 在吞吐量的性能测试中，发现我们的两种模型和`Muduo`之间有较大差距，**特别是`Burger::TcpServer`同样作为`Reactor`模型，性能只有`Muduo`的$70\%$**

### 解决过程
通过`Callgrind` +` Kcachegrind`，查看了整个程序的调用流程和代价，发现瓶颈为:
* `Burger::TcpServer`: **`send()`时多次`retrieveAllAsString()`** 
    * 产生原因：当时为了方便RingBuffer的接入，**send时是先读出来再发送，而不是直接用指针进行读写；产生了不必要的数据复制。**
    * 处理办法：**普通Buffer采用指针读写；RingBuffer使用两段式指针读写（暂未实现，FIXME！）**
<br>
* `Burger::CoTcpServer`: **多次上树下树操作**
    * 产生原因：之前的模型中，事件一旦到来，我们就会下树，逻辑简单；然而这样会导致很频繁的上下树，极其影响性能。
    * 处理办法：**在整个协程执行完之后再进行下树；为了避免此时fd已经随`Conn`的析构而关闭，采用`ConnMap`延长其生命周期。**

### 结果
* `Burger::TcpServer`的**性能提升到与`Muduo`相当**。
* `Burger::CoTcpServer`**的性能提升了超过$50\%$，达到了`Muduo`的$90\%+$.**