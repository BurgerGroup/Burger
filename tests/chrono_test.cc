#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip> // put_time

void func1() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    std::cout << ss.str() << std::endl;
}

void func2() {
    uint64_t uptime = 1614265476594215;
    std::time_t in_time_t = uptime / 1000000;
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    std::cout << ss.str() << std::endl;
}

int main() {
    // func1();
    func2();
}

