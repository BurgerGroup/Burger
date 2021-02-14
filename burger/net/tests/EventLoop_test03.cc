#include "burger/net/EventLoop.h"
#include <iostream>
/**
 * 主线程创建两个EventLoop对象
 * 负面测试
 */

using namespace burger;
using namespace burger::net;


int main() {
    EventLoop loop1;
    EventLoop loop2;
    return 0;
}