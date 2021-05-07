
#include "client.h"


Client::Client(const char* ip, uint16_t port) :
    port_(port) {
    struct sockaddr_in servaddr;
    connfd_ = socket(AF_INET, SOCK_STREAM, 0); 
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port_);
    inet_pton(AF_INET, ip, &servaddr.sin_addr); 
    int connect_rt = connect(connfd_, 
                    reinterpret_cast<struct sockaddr *>(&servaddr), 
                    static_cast<socklen_t>(sizeof(servaddr)));
    if(connect_rt < 0) {
        std::cout << " connect failed" << std::endl;
    }
}

void Client::send_data(int msgSize) {
    std::vector<char> query(msgSize, 'a'); 

    const char* cp = query.data();
    size_t remaining = query.size();
    while (remaining) {
        // 在阻塞I/O的情况下，send函数将阻塞直至将数据全部发送至发送缓存区，此种情况下，n_written等于remaining； 
        // 而在非阻塞I/O的情况下，send函数是"能写多少写多少"，所以n_written就不等于remaining了，而send_data函数为了同时对阻塞I/O和非阻塞I/O起作用，就用while循环了
        ssize_t n_written = send(connfd_, cp, remaining, 0);
        // 最后才打印下面这句话，说明send这里一直是阻塞的
        // 也就是说阻塞式套接字最终发送返回的实际写入字节数和请求字节数是相等的。
        // 非阻塞的后面再看
        std::cout << "send into buffer " << n_written << std::endl;

        if (n_written <= 0) {
            std::cout << "send failed" << std::endl;
            return;
        }
        n_written = static_cast<size_t>(n_written);
        remaining -= n_written;
        cp += n_written;
    }
    return;
}
