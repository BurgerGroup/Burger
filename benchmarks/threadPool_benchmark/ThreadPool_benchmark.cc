#include "burger/base/Threadpool.h"
#include <iostream>
#include <thread>
#include <functional>
#include <chrono>
#include <atomic>
#include <mutex>
#define BOOST
#include "threadpool.hpp"


using namespace burger;
using namespace std::chrono;

std::atomic<std::uint64_t> counter(0);
const size_t maxQueueSize = 100;
std::mutex global_mutex;

void task()
{
    std::this_thread::sleep_for(milliseconds(2));
    ++counter;
}

int main()
{
    Threadpool Pool("Test Pool");
    // Pool.setMaxQueueSize(maxQueueSize);
    Pool.start(100);

    auto begin = system_clock::now();
    for(int i = 0;i < 100000;++i)
    {
        Pool.run(task);
    }

    for(int i = 0;i < 200000;++i)
    {
        Pool.run(task);
    }

    while(counter < 300000){};
    auto end = system_clock::now();

    auto duration = duration_cast<milliseconds>(end - begin);
    std::cout << "Burger::ThreadPool:" << std::endl;
    std::cout << "任务数量：" << counter << std::endl;
	std::cout << "执行时间：" << duration.count() << " ms"<< std::endl;


    // boost 
    boost::threadpool::thread_pool<> threadPool(100);
    begin = system_clock::now();
    counter = 0;
    for(int i = 0;i < 100000;++i)
    {
        threadPool.schedule(task);
    }

    for(int i = 0;i < 200000;++i)
    {
        threadPool.schedule(task);
    }

    while(counter < 300000){};
    end = system_clock::now();

    duration = duration_cast<milliseconds>(end - begin);
    std::cout << "Boost::thread_pool<>:" << std::endl;
    std::cout << "任务数量：" << counter << std::endl;
	std::cout << "执行时间：" << duration.count() << " ms"<< std::endl;
    return 0;
}