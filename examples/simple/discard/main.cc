#include "discard.h"

#include "burger/base/Log.h"
#include "burger/net/InetAddress.h"

int main() {
    LOGGER(); LOG_LEVEL_TRACE;
    InetAddress listenAddr(8888);
    DiscardServer server(listenAddr);
    server.start();
}