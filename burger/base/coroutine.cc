#include "coroutine.h"
#include "config.h"

using namespace burger;


static std::atomic<uint64_t> s_CoId {0};
static std::atomic<uint64_t> s_CoNum {0};

static thread_local Coroutine* t_co = nullptr;
static thread_local Coroutine::ptr t_threadCo = nullptr;  // master Co ??why

static size_t g_coStackSize = Config::Instance().getSize("coroutine", "stackSize", 1024 * 1024);


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
    if(getcontext(&ctx_)) {
        BURGER_ASSERT2(false, "getcontext");
    }
    ++s_CoNum;
    DEBUG("Coroutine:Coroutine main");
}

// 有参构造才是真正创建线程
Coroutine::Coroutine(CallBack cb, size_t stackSize, bool mainSched)
    : coId_(++s_CoId),
    callback_(cb) {
    ++s_CoNum;
    stackSize_ = static_cast<uint32_t>(stackSize? stackSize : g_coStackSize);
    stack_ = StackAllocator::Alloc(stackSize_);
    if(getcontext(&ctx_)) {
        BURGER_ASSERT2(false, "getcontext");
    }
    ctx_.uc_link = nullptr;
    ctx_.uc_stack.ss_sp = stack_;
    ctx_.uc_stack.ss_size = stackSize_;
    if(mainSched) {
        makecontext(&ctx_, &Coroutine::MainFunc, 0);
    } else {
        makecontext(&ctx_, &Coroutine::CallMainFunc, 0);
    }

    DEBUG("Coroutine::Coroutine coId = {}", coId_);
}

Coroutine::~Coroutine() {
    --s_CoNum;
    if(stack_) {
        BURGER_ASSERT(state_ == State::TERM 
            || state_ == State::EXCEPT 
            || state_ == State::INIT);
        StackAllocator::Dealloc(stack_, stackSize_);    
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
    if(getcontext(&ctx_)) {
        BURGER_ASSERT2(false, "getcontext");
    } 
    ctx_.uc_link = nullptr;
    ctx_.uc_stack.ss_sp = stack_;
    ctx_.uc_stack.ss_size = stackSize_;
    makecontext(&ctx_, &Coroutine::MainFunc, 0);
    state_ = State::INIT;
}

void Coroutine::swapIn() {
    SetThis(this);
    BURGER_ASSERT(state_ != State::EXEC);
    state_ = State::EXEC;
    // if(swapcontextt(&Scheduler::GetMainCo()->ctx_, &ctx_)) {
    //     BURGER_ASSERT2(false, "swapcontextt");
    // }
}

void Coroutine::swapOut() {
    // SetThis(Scheduler::GetMainCo());
    // if(swapcontextt(&ctx_, &Scheduler::GetMainCo()->ctx_)) {
    //     BURGER_ASSERT2(false, "swapcontextt");
    // }
}

void Coroutine::call() {
    SetThis(this);
    state_ = State::EXEC;
    if(swapcontext(&t_threadCo->ctx_, &ctx_)) {
        BURGER_ASSERT2(false, "swapcontextt");
    }
}

void Coroutine::back() {
    SetThis(t_threadCo.get());
    if(swapcontext(&ctx_, &t_threadCo->ctx_)) {
        BURGER_ASSERT2(false, "swapcontextt");
    }
}

void Coroutine::SetThis(Coroutine* f) {
    t_co = f;
}

Coroutine::ptr Coroutine::GetThis() {
    if(t_co) {
        return t_co->shared_from_this();
    }
    Coroutine::ptr mainCo = std::shared_ptr<Coroutine>();
    BURGER_ASSERT(t_co == mainCo.get());
    t_threadCo = mainCo;
    return t_co->shared_from_this();
}

void Coroutine::YeildToReady() {
    Coroutine::ptr cur = GetThis();
    BURGER_ASSERT(cur->state_ == State::EXEC);
    cur->state_ = State::READY;
    cur->swapOut();
}

void Coroutine::YeildToHold() {
    Coroutine::ptr cur = GetThis();
    BURGER_ASSERT(cur->state_ == State::EXEC);
    // cur->state_ = State::HOLD;
    cur->swapOut();
}

uint64_t Coroutine::getCoNums() {
    return s_CoNum;
}

// 此两个函数可以优化
void Coroutine::MainFunc() {
    Coroutine::ptr cur = GetThis();
    BURGER_ASSERT(cur);
    try {
        cur->callback_();
        cur->callback_ = nullptr;
        cur->state_ = State::TERM;
    } catch(std::exception& ex) {
        cur->state_ = State::EXCEPT;
        ERROR("Coroutine Except : {} coId = {} \n {}",
             ex.what(), cur->getCoId(), util::BacktraceToString());
    } catch(...) {
        cur->state_ = State::EXCEPT;
        ERROR("Coroutine Except coID = {} \n {}", 
            cur->getCoId(), util::BacktraceToString());
    }
    auto rawPtr = cur.get();
    cur.reset();
    rawPtr->swapOut();
    BURGER_ASSERT2(false, "never reach coId = " + std::to_string(rawPtr->getCoId()));
}

void Coroutine::CallMainFunc() {
    Coroutine::ptr cur = GetThis();
    BURGER_ASSERT(cur);
    try {
        cur->callback_();
        cur->callback_ = nullptr;
        cur->state_ = State::TERM;
    } catch(std::exception& ex) {
        cur->state_ = State::EXCEPT;
        ERROR("Coroutine Except : {} coId = {} \n {}",
             ex.what(), cur->getCoId(), util::BacktraceToString());
    } catch(...) {
        cur->state_ = State::EXCEPT;
        ERROR("Coroutine Except coID = {} \n {}", 
            cur->getCoId(), util::BacktraceToString());
    }
    auto rawPtr = cur.get();
    cur.reset();
    rawPtr->back();
    BURGER_ASSERT2(false, "never reach coId = " + std::to_string(rawPtr->getCoId()));
}






 



