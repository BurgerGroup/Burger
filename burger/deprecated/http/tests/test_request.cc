#include <iostream>
#include "burger/http/HttpRequest.h"

using namespace burger;
using namespace burger::http;

int main() {
    HttpRequest::ptr req = std::make_shared<HttpRequest>();
    req->setHeader("host" , "www.burger.top");
    req->setBody("hello burger");
    req->dump(std::cout) << std::endl;

}