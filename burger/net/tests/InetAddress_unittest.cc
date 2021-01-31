#include "burger/net/InetAddress.h"
#include "gtest/gtest.h"
#include <iostream>
using namespace burger;
using namespace burger::net;

TEST(InetAddress, constructor) {
    InetAddress addr0(11234);
    EXPECT_EQ(addr0.getPort(), 11234);
    EXPECT_EQ(addr0.getPortStr(), std::string("11234"));
    EXPECT_EQ(addr0.getIpStr(), std::string("0.0.0.0"));
    EXPECT_EQ(addr0.getIpPortStr(), std::string("0.0.0.0:11234"));

    InetAddress addr1("1.2.3.4", 8888);
    EXPECT_EQ(addr1.getPort(), 8888);
    EXPECT_EQ(addr1.getPortStr(), std::string("8888"));
    EXPECT_EQ(addr1.getIpStr(), std::string("1.2.3.4"));
    EXPECT_EQ(addr1.getIpPortStr(), std::string("1.2.3.4:8888"));

    struct sockaddr_in addr2;
    addr2.sin_family = AF_INET;
    addr2.sin_port = sockets::hostToNetwork16(11111);
    addr2.sin_addr.s_addr = sockets::hostToNetwork32(INADDR_ANY);
    InetAddress addr3(addr2);
    EXPECT_EQ(addr3.getPort(), 11111);
    EXPECT_EQ(addr3.getPortStr(), std::string("11111"));
    EXPECT_EQ(addr3.getIpStr(), std::string("0.0.0.0"));
    EXPECT_EQ(addr3.getIpPortStr(), std::string("0.0.0.0:11111"));
}


// TODO
// TEST(testInetAddressResolve, resolve) {
//     InetAddress addr(80);
//     if (InetAddress::hostNameToIp("google.com", &addr))
//     {
//         LOG_INFO << "google.com resolved to " << addr.toIpPort();
//     }
//     else
//     {
//         LOG_ERROR << "Unable to resolve google.com";
//     }
// }


// int main() {
//     InetAddress addr0(11234);
//     std::cout << addr0.getPortStr() << std::endl;
//     std::cout << addr0.getPortStr() == std::string(11234) << std::endl;
// }