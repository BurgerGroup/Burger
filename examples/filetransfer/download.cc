// 一次性把文件读入内存(string)，一次性调用send发送完毕
// 这种不满足 "内存消耗只和并发连接数有关，跟文件大小无关"


#include "burger/net/Scheduler.h"
#include "burger/net/CoTcpServer.h"

#include <string>
#include <stdio.h>
#include <unistd.h>

using namespace burger;
using namespace burger::net;

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

