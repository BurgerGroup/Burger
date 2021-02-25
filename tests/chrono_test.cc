#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip> // put_time

int main() {
    uint64_t uptime = 1614265476594215;
    std::time_t in_time_t = uptime / 1000000;
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    std::cout << ss.str() << std::endl;
}

