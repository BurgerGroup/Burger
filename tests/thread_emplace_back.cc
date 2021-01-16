#include <thread>
#include <vector>
#include <algorithm>
#include <iostream>

void ThreadFunc() {
    std::cout << std::this_thread::get_id() << std::endl;
}

int main() {
    int numThreads = 10;
    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    for(int i = 0; i < numThreads; i++) {
        threads.emplace_back(std::thread(ThreadFunc));
    }
    std::for_each(threads.begin(), threads.end(), 
                std::mem_fn(&std::thread::join));    
}