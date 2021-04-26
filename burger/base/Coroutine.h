#ifndef COROUTINE_H
#define COROUTINE_H

#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>
#include <string>
#include <cassert>
#include "fcontext/fcontext.h"

namespace burger {

class Coroutine : public boost::noncopyable, public std::enable_shared_from_this<Coroutine> {
public:
    using Callback = std::function<void()>;
    using ptr = std::shared_ptr<Coroutine>;

    enum class State {
        EXEC,  // 可运行，包括初始化，从epoll_wait()中返回，从wait()从返回
        HOLD,  // 等待epoll中, 暂时没用
        TERM   // 运行结束
    };
    // why can't define private??
    Coroutine();
    Coroutine(const Callback& cb, std::string name = "anonymous", size_t stackSize = 0);
    ~Coroutine();

    static void SwapOut();  //切换到当前线程的主协程
    void swapIn(); //执行当前协程
    Callback getCallback() { return cb_; }
    std::string getName() const { return name_; };
    void setState(State state) { state_ = state; };
    State getState() { return state_; }

    uint64_t getCoId() const { return coId_; }
    static uint64_t GetCoId();
    static Coroutine::ptr GetCurCo();
    static Coroutine::ptr GetMainCo();
    static void SetThisCo(Coroutine* co); // 色湖之当前线程的运行协程
    void termiate();
private:
    static void RunInCo(intptr_t vp);
    void checkCurCo();

    uint64_t coId_;
    size_t stackSize_ = 0; 
    State state_;
    std::string name_;
    fcontext_t ctx_;
    void* stack_ = nullptr;
    Callback cb_ = nullptr;
};

} // namespace burger

#endif // COROUTINE_H
