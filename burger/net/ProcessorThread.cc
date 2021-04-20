#include "ProcessorThread.h"
#include "Processor.h"
using namespace burger;
using namespace burger::net;

ProcessThread::ProcessThread(Scheduler* scheduler) 
    : scheduler_(scheduler) {
}

ProcessThread::~ProcessThread() {
    thread_.join();
}

Processor* ProcessThread::startProcess() {
    thread_ = std::thread{&ProcessThread::threadFunc, this};
    Processor* proc = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]{ return proc_ != nullptr; });
        proc = proc_;
    }
    return proc;
}

void ProcessThread::threadFunc() {
    Processor proc;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        proc_ = &proc;
        cv_.notify_one();
    }
    proc.run();
    
    std::lock_guard<std::mutex> lock(mutex_);
    proc_ = nullptr;
}


