#include "burger/base/Util.h"
#include <thread>
#include <iostream>

using namespace burger;

void printTid() {
    std::cout << util::tid() << std::endl; 
}

int main() {
    printTid();
    std::thread t1(printTid);
    std::thread t2(printTid);
    t1.join();
    t2.join();
}