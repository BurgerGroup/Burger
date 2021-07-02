#include "burger/http/HttpStatus.h"
#include <iostream>

using namespace burger;
using namespace burger::http;

int main() {
    const char* s = HttpStatusToString(HttpStatus::NOT_FOUND);
    std::cout << s << std::endl;
}
