## TcpConnection 的生命周期管理

在Channel中维护一个weak_ptr(tie_), 将这个shared_ptr对象赋值给tie_, 计数依旧为1

当连接关闭，调用handleEvent(), 将tie_提升，得到一个shared_ptr对象，计数2


1. 当有一个新的连接产生 : TcpServer::newConnection

创建一个TcpConnection对象(shared_ptr管理)，引用计数为1

装入connectionsMap_, 引用计数为2

2. 创建之后，建立连接，TcpConnection::connectEstablished()

这里调用了ConnectionCallback()

并将TcpConnection的Channel上树关注TcpConnection的动态

创建完后并连接后，只有connectionMap_存有一个连接计数

3. 当这个TcpConnection有事件到来，handleEvent - handleRead 

进入handleEvent, tie_提升1个计数

当有消息，是onMessage

当是close要断开连接的情况 handleclose()

4. handleClose

handleClose里有临时变量,引用计数暂时到3,

调用注册的TcpServer::removeConnection,

connectionsMap_ erase掉变成2

执行connectDestroyed因为放进了queue,变长3

执行完弹出回到handleClose

再弹出少一个临时变量guardThis 变长2

回到handleEvent, 执行完少一个临时变量guard,变成1

这个1就是在functors里面,直到他执行完后才销毁, 刚好结束生命周期

**感觉被动关闭,tie_提升也没啥用, todo 测试下主动关闭**