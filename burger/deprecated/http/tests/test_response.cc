#include "burger/http/HttpResponse.h"
#include <iostream>

using namespace burger;
using namespace burger::http;

int main() {
    HttpResponse::ptr rsp = std::make_shared<HttpResponse>();
    rsp->setHeader("X-X", "burger");
    rsp->setBody("hello burger");
    rsp->setStatus(static_cast<HttpStatus>(400));
    rsp->setClose(false);

    rsp->dump(std::cout) << std::endl;
}