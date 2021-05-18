// 一次性把文件读入内存(string)，一次性调用send发送完毕
// 这种不满足 "内存消耗只和并发连接数有关，跟文件大小无关"


#include "burger/net/Scheduler.h"
#include "burger/net/CoTcpServer.h"
#include "burger/base/Log.h"
#include <string>
#include <stdio.h>
#include <unistd.h>

using namespace burger;
using namespace burger::net;

const char* g_file = nullptr;

std::string readFile(const char* filename) {
    std::string content;
    FILE* fp = ::fopen(filename, "rb");
    if (fp) {
        // inefficient!!!
        const int kBufSize = 1024*1024;
        char iobuf[kBufSize];
        ::setbuffer(fp, iobuf, sizeof iobuf);

        char buf[kBufSize];
        size_t nread = 0;
        while ( (nread = ::fread(buf, 1, sizeof buf, fp)) > 0) {
            content.append(buf, nread);
        }
        ::fclose(fp);
    }
    return content;
}

void connHandler(const CoTcpConnectionPtr& conn) {
    INFO("File Server sending file {} to {}", g_file, conn->getPeerAddr().getIpPortStr());
    std::string fileContent = readFile(g_file);
    conn->send(fileContent);
    conn->shutdown();
    INFO("FileServer send done ...");
}


int main(int argc, char* argv[]) {
    if (argc > 1) {
        g_file = argv[1];

        Scheduler sched;
        InetAddress listenAddr(8888);
        CoTcpServer server(&sched, listenAddr, "FileServer");
        server.setConnectionHandler(connHandler);
        server.start();
        sched.wait();
    }
    else {
        fprintf(stderr, "Usage: %s file_for_downloading\n", argv[0]);
    }
}
