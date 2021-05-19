#include "burger/net/Scheduler.h"
#include "burger/net/CoTcpServer.h"
#include "burger/base/Log.h"

#include <stdio.h>
#include <unistd.h>
#include <memory>

using namespace burger;
using namespace burger::net;

const int kBufSize = 64*1024;  // 64K
const char* g_file = NULL;
using FilePtr = std::shared_ptr<FILE>;

void connHandler(const CoTcpConnectionPtr& conn) {
    INFO("FileServer - Sending file {} to {}", g_file, conn->getPeerAddr().getIpPortStr());
    FILE* fp = ::fopen(g_file, "rb");
    FilePtr fptr(fp, ::fclose);    // wrap资源和对象绑定到一起
    if (fptr) {
        char buf[kBufSize];
        size_t nread = 0;
        do {
            nread = ::fread(buf, 1, sizeof buf, fp);
            conn->send(buf, nread);
        } while(nread > 0);

        conn->shutdown();
        INFO("FileServer - done");
    } else {
        conn->shutdown();
        INFO("FileServer - no such file");
    }
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