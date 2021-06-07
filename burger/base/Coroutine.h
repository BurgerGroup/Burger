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
        EXEC,  // 运行状态
        HOLD,  // 挂起状态
        TERM   // 未执行状态
    };
    // why can't define private??
    Coroutine();
    Coroutine(const Callback& cb, std::string name = "", size_t stackSize = 0);
    ~Coroutine();

    static void Yield();  //切换到当前线程的主协程
    void resume(); //执行当前协程
    Callback getCallback() { return cb_; }
    std::string getName() const { return name_; }; 
    State getState() { return state_; }
    void setState(State state) { state_ = state; };
    void setCallback(const Callback& cb) { cb_ = cb; }
    void setName(const std::string& name) { name_ = name; }; 
    void setFd(int fd) { fd_ = fd; }
    int getFd() const { return fd_; }

    uint64_t getCoId() const { return coId_; }
    static uint64_t GetCoId();
    static Coroutine::ptr GetCurCo();
    static Coroutine::ptr GetMainCo();
    static void SetThisCo(Coroutine* co); // 设置当前线程的运行协程
    void termiate();
    void reset(const Callback& cb);
private:
    static void RunInCo(intptr_t vp);
    static void ProduceMainCo();

    uint64_t coId_;
    size_t stackSize_ = 0; 
    State state_;
    std::string name_;
    fcontext_t ctx_;
    void* stack_ = nullptr;
    Callback cb_ = nullptr;
    int fd_ = -1;
};

} // namespace burger

#endif // COROUTINE_H
