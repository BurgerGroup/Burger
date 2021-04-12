# Buffer的实现与思考

## 1.传统Buffer
以字符数组为底层的`Buffer`实现，以`reader`、`writer`指针标记读、写位置，并基于此实现各种功能：
```cpp
/// +----------------------+-------------------------------------+
/// |      blank part      |  readable bytes  |  writable bytes  |
/// |(CONTENT is read away)|     (CONTENT)    |                  |
/// +----------------------+------------------+------------------+
///                        |                  |                  |
/// 0        <=       readerIndex   <=   writerIndex    <=     size
```

* 使用下标代替裸指针，避免数据搬移引起的指针失效

## 2.Muduo::net::Buffer
以`std::vector<char>`作为底层：
```cpp
/// +-------------------+-------------------------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0       <=     readerIndex   <=   writerIndex    <=     size
```
* 在传统的基础上添加了`prepend`功能，使得可以在不搬移数据的情况下在字符串开头添加一定长度的数据。

* 在**写指针到达末尾**时，会**检查读指针前是否有大量空白**；如果是则不会重新申请空间，而是**在原空间进行数据搬移，也叫内部腾挪（这催生了我们利用环形Buffer来避免数据搬移的想法）**。


## 3.环形Burger::RingBuffer
我们希望利用环形缓冲区来避免数据搬移：
传统`Buffer`在写指针到达末尾后，**除非进行内部腾挪，否则是不能继续写的**；而环形`Burger::RingBuffer`可以**无需内部腾挪，直接在缓冲区头部继续写数据**。

```cpp
/// +------------------+------------------+-----------------+------------------+
/// |  readable bytes  |  writable  bytes |   min_prepend   |  readable bytes  |
/// |    (CONTENT)     |                  | (kCheapPrepend) |    (CONTENT)     |
/// |                  |          prependable bytes         |                  |
/// +------------------+------------------------------------+------------------+
/// |                  |                                    |                  |
/// 0      <=     writerIndex                          readerIndex   <=       size
```

实现的环形`Burger::RingBuffer`有以下要点：

* 在计算`writableBytes`时**务必留出Prepend所需的最小空间**（这里是8bytes）；当**无法留出足够的Prepend空间时，则状态为不可写**

* 在寻找`CRLF("\r\n")`时，需要考虑其**在末尾被截断**的情况

* 在读取`int16～int64`时，同样需要考虑其**在末尾被截断**的情况；这种情况需要我们**分别读取两部分后再拼接**。

* 无论是正常情况（**reader指针在writer指针之前**）还是环形情况（**reader指针在writer指针之后**），我们都需要保证reader指针前有一定的`prepend`区域（否则，考虑写满时，若`reader指针`在中间，就无法`prepend`）；

* 这就要求writer转回时，不是从头开始，而是需要在头部留出空白，否则**如果reader指针正好停在整个Buffer的最开始处时，则无法进行prepend**，如下图所示：
```cpp
/// +-----------------------+------------------+-----------------+
/// |   readable bytes      |  writable  bytes |   min_prepend   |
/// |      (CONTENT)        |                  | (kCheapPrepend) |
/// |                       |          prependable bytes         |
/// +-----------------------+------------------------------------+
/// |                       |                                    |
/// 0(readerIndex)  <=  writerIndex            <=              size
```

* 要解决图示的问题，则**需要在整个Buffer前再设置一个Prepend区域**；(但我并不喜欢这样麻烦且不优雅的设计。还有更好的解决方案吗？)

## 4.Burger::IBuffer
IBuffer类似于一个抽象基类，用户可以继承此基类来实现自己所需的Buffer（当然需要实现要求的接口），就像`Burger::RingBuffer`和`Burger::Buffer`都是其子类。

* 环形`Burger::RingBuffer`**只在内部腾挪次数多的场景下有明显优势**，如果整个操作过程完全不涉及到内部腾挪，那么和传统Buffer并无差别，甚至由于各种截断拼接判断还会略慢一些。这也是我们保留`Burger::Buffer`的原因。