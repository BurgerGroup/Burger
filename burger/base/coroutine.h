#ifndef COROUTINE_H
#define COROUTINE_H

#include <ucontext.h>
#include <thread>
#include <memory>
#include <functional>
#include <atomic>
#include "util.h"
#include "Log.h"

namespace burger {

class Coroutine : public std::enable_shared_from_this<Coroutine> {
public:
    using ptr = std::shared_ptr<Coroutine>;
    using CallBack = std::function<void()>;
    enum class State {
        INIT,  // 初始化状态
        HOLD, // 暂停状态
        EXEC,  // 执行中状态
        TERM,  // 结束状态
        READY,  // 可执行状态
        EXCEPT  // 异常状态
    };

    Coroutine(CallBack cb, size_t stackSize = 0, bool mainSched = false);
    ~Coroutine();
    void reset(CallBack cb);
    void swapIn();   // 切换到当前协程执行
    void swapOut();  // 切换到后台执行 
    void call();        // 将当前协程切换到执行状态
    void back();     //  将当前协程切换到后台
    static void SetThis(Coroutine* f);   // 设置当前线程的运行协程
    static Coroutine::ptr GetThis();   // 返回当前协程
    static void YeildToReady();      // 协程切换到后台，并设置程Ready状态
    static void YeildToHold();      // 协程切换到后台，并设置为Hold状态
    static uint64_t getCoNums();   
    static void MainFunc();   // 执行完成返回到线程主协程
    static void CallMainFunc();   // 执行完成返回到线程调度协程
    uint64_t getCoId() const { return coId_; }
    static uint64_t GetCoId();
    void setState(State state) { state_ = state; }
    State getState() const { return state_; }

private:
    Coroutine();  // 每个线程第一个协程的构造
    uint64_t coId_ = 0;
    uint32_t stackSize_ = 0;
    State state_ = State::INIT;
    ucontext_t ctx_; // 协程上下文
    void* stack_ = nullptr; // 协程运行栈指针
    CallBack callback_;      // 协程运行函数
};

} // namespace burge 



#endif // COROUTINE_H