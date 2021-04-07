#include "coroutine.h"
#include "config.h"

using namespace burger;


static std::atomic<uint64_t> s_CoId {0};
static std::atomic<uint64_t> s_CoNum {0};

static thread_local Coroutine* t_co = nullptr;
static thread_local std::shared_ptr<Coroutine::ptr> t_threadCo = nullptr;  // master Co ??why

static int g_coStackSize = Config::Instance().getSize("coroutine", "stackSize", 1024 * 1024);


// todo : RAII
class MallocStackAllocator {
public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }

    static void Dealloc(void* vp, size_t size) {
        return free(vp);
    }
};

using StackAllocator = MallocStackAllocator;


Coroutine::Coroutine() {
    state_  = State::EXEC;
    SetThis(this);
    // get到当前线程的上下文
    if(getConetext(&ctx_)) {
        BURGER_ASSERT2(false, "getcontext");
    }
    ++s_CoNum;
    DEBUG("Coroutine:Coroutine main");
}

// 有参构造才是真正创建线程
Coroutine::Coroutine(CallBack cb, size_t stackSize, bool mainSched)
    : coId_(++s_CoId),
    callback_(cb) {
    ++s_CoNum++;
    stackSize_ = stackSize? stackSize : g_coStackSize;
    stack_ = StackAllocator::Alloc(stackSize_);
    if(getConetext(&ctx_)) {
        BURGER_ASSERT2(false, "getcontext");
    }
    ctx_.uc_link = nullptr;
    ctx_.uc_stack.ss_sp = stack_;
    ctx_.uc_stack.ss_size = stackSize_;
    if(mainSched) {
        makecontex(&ctx_, &Coroutine::MainFunc, 0);
    } else {
        makecontex(&ctx_, &Coroutine::CallMainFunc, 0);
    }

    DEBUG("Coroutine::Coroutine coId = {}", coId_);
}

Coroutine::~Coroutine() {
    --s_CoNum;
    if(stack_) {
        BURGER_ASSERT(state_ == State::TERM 
            || state_ == State::EXCEPT 
            || state_ == State::INIT);
        StackAllocator::Dealloc(m_stack, m_stacksize);    
    } else {
        BURGER_ASSERT(!callback_);
        BURGER_ASSERT(state_ == State::EXEC);
        Coroutine* cur = t_co;
        if(cur == this) {
            SetThis(nullptr);
        }
    }
    DEBUG("Coroutine::~Coroutine coId = {} total = {}", coId_, s_CoNum );
}

void Coroutine::reset(CallBack cb) {
    BURGER_ASSERT(stack_);
    BURGER_ASSERT(state_ == State::TERM 
        || state_ == State::EXCEPT 
        || state_ == State::INIT);
    callback_ = cb;
    if(getConetext(&ctx_)) {
        BURGER_ASSERT2(false, "getcontext");
    } 
    ctx_.uc_link = nullptr;
    ctx_.uc_stack.ss_sp = stack_;
    ctx_.uc_stack.ss_size = stackSize_;
    makecontex(&ctx_, &Coroutine::MainFunc, 0);
    state_ = State::INIT;
}

void Coroutine::swapIn() {
    SetThis(this);
    
}

void Coroutine::call() {
    SetThis(this);
    state_ = State::EXEC;
    if(swapcontex(&t_threadCo->ctx_, &ctx_)) {
        BURGER_ASSERT2(false, "swapcontext");
    }
}

void Coroutine::back() {
    SetThis(t_threadCo.get());
    if(swapcontex(&ctx_, &t_threadCo->ctx_)) {
        BURGER_ASSERT2(false, "swapcontext");
    }
}

 



