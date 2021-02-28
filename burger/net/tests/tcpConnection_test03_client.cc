#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>

/**
 * 等到10s后客户端关闭，观察server的log，HUP IN才来
 */


#define SERV_PORT 8888 // 要和服务器严格对应起来

void sys_err(const char* str) {
    perror(str);
    exit(1);
}

int main(int argc, char *argv[]) {
    int cfd;
    char buf[1024];
    struct sockaddr_in serv_addr;  // 服务器地址结构
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);
    
    cfd = socket(AF_INET, SOCK_STREAM, 0);
    if(cfd == -1) {
        perror("socket error");
    }
    int connect_ret = connect(cfd, reinterpret_cast<struct sockaddr *>(&serv_addr), sizeof(serv_addr));

    if(connect_ret != 0) {
        sys_err("connect error");
    }
    while(true) {
        ssize_t ret = read(cfd, buf, sizeof(buf));
        std::cout << "ret = " << ret << std::endl;
        if(ret == 0) break;
        std::cout << buf << std::endl;
        bzero(buf, sizeof(buf));
    }
    sleep(10);
    close(cfd);
    
    return 0;
}