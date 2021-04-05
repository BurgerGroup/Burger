#ifndef BEEF_H
#define BEEF_H

#include <functional>
#include <string>
#include "HttpServer.h"

namespace burger {
namespace http {
    
class Beef {
public:
    using Method = std::function<std::string()>;
    Beef() = default;
    void handle() {}
    void route(const std::string& url, Method m) {}
    Beef& port(std::uint16_t port) {
        port_ = port;
        return *this;
    }
    void run() {
        EventLoop loop;
        HttpServer<Beef> server(this, &loop, InetAddress(port_), "dummy");
        // server.setThreadNum(numThreads_);
        server.setThreadNum(1);
        server.start();
        loop.loop();
    }
private:
    uint16_t port_ = 80;
    // int numThreads_;
};
} // namespace http
} // namespace burger



#endif // BEEF_H