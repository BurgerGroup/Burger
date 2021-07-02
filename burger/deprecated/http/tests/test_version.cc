#include "burger/http/Version.h"
#include <iostream>

using namespace burger;
using namespace burger::http;

int main() {
    std::cout << httpVersionTostr(Version::kHttp10) << std::endl;
    std::cout << httpVersionTostr(Version::kHttp11) << std::endl;

}