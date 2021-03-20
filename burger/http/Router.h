#ifndef ROUTER_H
#define ROUTER_H

#include <unordered_map>
#include <string>
#include "burger/base/ThreadPool.h"

namespace burger {
namespace http {

class HttpRequest;
class HttpResponse;

using request_handler = std::function<void(const std::shared_ptr<HttpRequest>&, const std::shared_ptr<HttpResponse>&)>;


class router {
public:
    router() = default;
    ~router();

    void run(int work_threads);
    std::size_t route_table_size() const { return routeTable_.size(); }
    void route(const std::string& uri, const request_handler& func);
    void stop();
    void do_route(const std::shared_ptr<request>& req, const std::shared_ptr<response>& res);

private:
    void route_thread(const std::shared_ptr<request>& req, const std::shared_ptr<response>& res);

private:
    Threadpool threadPool_;
    std::unordered_map<std::string, request_handler> routeTable_;
};

} // namespace http

} // namespace burge 


class route {

}


#endif // ROUTER_H