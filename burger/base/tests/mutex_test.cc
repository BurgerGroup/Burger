
#include <thread>
#include <iostream>
#include <mutex>
#include <vector>
#include <algorithm>
#include "burger/base/Timestamp.h"

using namespace burger;

std::mutex g_mutex;
std::vector<int> g_vec;
const int kCount = 10 * 1000 * 1000;

void ThreadFunc() {
    for(int i = 0; i < kCount; i++) {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_vec.push_back(i);
    }
}

int main() {
    const int kMaxThreads = 8;
    g_vec.reserve(kMaxThreads * kCount);
    Timestamp start(Timestamp::now());
    for(int i = 0; i < kCount; i++) {
        g_vec.push_back(i);
    }

    std::cout << "Single thread without lock\t" << timeDifference(Timestamp::now(), start) << std::endl;

    start = Timestamp::now();
    ThreadFunc();
    std::cout << "Single thread with lock\t" << timeDifference(Timestamp::now(), start) << std::endl;

    for(int i = 1; i < kMaxThreads; i++) {
        std::vector<std::thread> threads;
        g_vec.clear();
        start = Timestamp::now();
        for(int j = 0; j < i; j++) {
            threads.push_back(std::thread(ThreadFunc));
        }
        std::for_each(threads.begin(), threads.end(),
                    std::mem_fn(&std::thread::join));
        
        std::cout << i <<" thread(s) with lock\t" << timeDifference(Timestamp::now(), start) << std::endl;

    }

}