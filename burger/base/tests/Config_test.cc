#include "burger/base/Config.h"

using namespace burger;

int main() {
    std::cout << Config::Instance().getString("mysql", "host", "127.0.0.1")  << std::endl;
    std::cout << Config::Instance().getString("mysql", "user", "ysy") << std::endl;
    std::cout << Config::Instance().getString("mysql", "password", "123") << std::endl;
    std::cout << Config::Instance().getString("mysql", "dbname") << std::endl;
    // std::cout << Config::Instance().getDouble("mysql", "test") << std::endl;
    // todo : 如何读浮点数表达式？
    std::cout << Config::Instance().getInt("coroutine", "stackSize", 1024 * 1024) << std::endl;
    std::cout << Config::Instance().getString("test", "test1", "null") << std::endl;
    std::cout << Config::Instance().getInt("test", "test1", 0) << std::endl;

    std::cout << Config::Instance().getUInt16("rpc", "rpcServerPort", 8000) << std::endl;

}