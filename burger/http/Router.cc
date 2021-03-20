#include "Router.h"
#include "HttpStatus.h"

using namespace burger;
using namespace burger::http;

router::~router() {
    stop();
}

void router::run(int work_threads) {
    threadPool_.start(work_threads);
}

void router::route(const std::string& uri, const request_handler& func) {
    routeTable_.emplace(uri, func);
}

void router::stop() {
    threadPool_.stop();
    routeTable_.clear();
}

void router::do_route(const std::shared_ptr<request>& req, const std::shared_ptr<response>& res) {
    threadPool_.run(std::bind(&router::route_thread, this, req, res));
}

void router::route_thread(const std::shared_ptr<request>& req, const std::shared_ptr<response>& res) {
    auto it = route_table_.find(req->uri); // todo get uri
    if (it != route_table_.end()) {
        try {
            it->second(req, res);
        } catch (std::exception& e) {
            WARN("{}", e.what);

        }
    } else {
        res->set_response(HttpStatus::NOT_FOUND);
        WARN("Route failed, uri: {}", req->getUri());  // todo uri
    }
}






