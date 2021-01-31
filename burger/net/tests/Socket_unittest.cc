// #include "gtest/gtest.h"
#include "burger/net/Socket.h"

#include <iostream>

using namespace burger;
using namespace burger::net;

// TEST(Socket, getTcpInfoString) {
//     Socket(11);
// }

int main() {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    Socket sk(fd);
    std::cout << sk.getTcpInfoString() << std::endl;
}