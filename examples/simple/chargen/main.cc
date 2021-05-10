#include "chargen.h"
#include "burger/net/Scheduler.h"
#include "burger/base/Log.h"



// 只负责发送
// nc 127.0.0.1 8888 | head   只输出前10行 -20 前20行
// 看到吞吐量可以感受到 TCP 流量调节功能 : 即使服务端生成数据很快，但客户端接收或处理速度慢
int main() {
    LOGGER(); LOG_LEVEL_INFO;
    Scheduler sched;

    InetAddress listenAddr(8888);
    ChargenServer server(&sched, listenAddr);
    
    server.start();
    sched.wait();
}