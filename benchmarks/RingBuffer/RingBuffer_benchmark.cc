#include <benchmark/benchmark.h>
#include "burger/net/RingBuffer.h"
#include "burger/net/Buffer.h"
using namespace burger;
using namespace burger::net;
using namespace std;

string msg;
const int blockSize = 16384;

void rwBuffer(IBuffer& buf) {
    buf.append(msg);
    buf.retrieve(50);
    buf.append(std::string(42, 's'));
    std::string tmp(8, 's');
    buf.prepend(reinterpret_cast<const void*>(tmp.c_str()), 8); // 把前面填满
}

auto BM_RingBuffer = [](benchmark::State& state)->void {
    RingBuffer buf;
    for(auto _ : state) {
        buf.retrieveAll();
        rwBuffer(buf);
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(msg.length()));
    state.SetItemsProcessed(state.iterations());
};

auto BM_Buffer = [](benchmark::State& state)->void {
    Buffer buf;
    for(auto _ : state) {
        buf.retrieveAll();
        rwBuffer(buf);
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(msg.length()));
    state.SetItemsProcessed(state.iterations());
};

int main(int argc, char** argv) {
    for (int i = 0; i < blockSize; ++i) {
        msg.push_back(static_cast<char>(i % 128));
    }
    benchmark::RegisterBenchmark("RingBuffer", BM_RingBuffer)->Iterations(500000);
    benchmark::RegisterBenchmark("Buffer", BM_Buffer)->Iterations(500000);
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}