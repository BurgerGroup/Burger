#ifndef CLIENT_H
#define CLIENT_H


#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>
#include <fcntl.h>
#include <unistd.h> // close
#include <sys/types.h>  
#include <memory>
#include <iostream>
#include <cerrno> 
#include <vector>

class Client {
public:
    Client(const char* ip = "127.0.0.1", uint16_t port = 8888);
    ~Client() { close(connfd_); }

    void send_data(int msgSize = 102400);
private:
    int connfd_;
    uint16_t port_;
};


#endif // CLIENT_H