#include "burger/base/Config.h"

using namespace burger;

int main() {
    std::cout << Config::Instance().getString("mysql", "host", "127.0.0.1")  << std::endl;
    std::cout << Config::Instance().getString("mysql", "user", "ysy") << std::endl;
    std::cout << Config::Instance().getString("mysql", "password", "123") << std::endl;
    std::cout << Config::Instance().getString("mysql", "dbname") << std::endl;
    std::cout << Config::Instance().getDouble("mysql", "test") << std::endl;
    // todo : 如何读计算出来的数字
    std::cout << Config::Instance().getInt("coroutine", "stackSize", 1024 * 1024) << std::endl;
}