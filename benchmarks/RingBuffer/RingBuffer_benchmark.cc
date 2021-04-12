#include <benchmark/benchmark.h>
#include "burger/net/RingBuffer.h"
#include "burger/net/Buffer.h"
using namespace burger;
using namespace burger::net;
using namespace std;

string msg;
const int blockSize = 65536;

void rwBuffer(IBuffer& buf, size_t len = 50) {
    buf.append(msg);
    buf.retrieve(len);
    buf.append(std::string(len - 8, 's'));
    std::string tmp(8, 's');
    buf.prepend(reinterpret_cast<const void*>(tmp.c_str()), 8); // 把前面填满
}

auto BM_RingBuffer = [](benchmark::State& state)->void {
    size_t len = state.range(0);
    RingBuffer buf;
    for(auto _ : state) {
        buf.retrieveAll();
        rwBuffer(buf, len);
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(msg.length()));
    state.SetItemsProcessed(state.iterations());
};

auto BM_Buffer = [](benchmark::State& state)->void {
    size_t len = state.range(0);
    Buffer buf;
    for(auto _ : state) {
        buf.retrieveAll();
        rwBuffer(buf, len);
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(msg.length()));
    state.SetItemsProcessed(state.iterations());
};

int main(int argc, char** argv) {
    for (int i = 0; i < blockSize; ++i) {
        msg.push_back(static_cast<char>(i % 128));
    }
    benchmark::RegisterBenchmark("RingBuffer", BM_RingBuffer)->Iterations(500000)->Arg(static_cast<int>(blockSize * 0.03))->Arg(static_cast<int>(blockSize * 0.5))->Arg(static_cast<int>(blockSize * 0.95));
    benchmark::RegisterBenchmark("Buffer", BM_Buffer)->Iterations(500000)->Arg(static_cast<int>(blockSize * 0.03))->Arg(static_cast<int>(blockSize * 0.5))->Arg(static_cast<int>(blockSize * 0.95));
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}