#include "coroutine.h"
#include "config.h"
#include "scheduler.h"

using namespace burger;

static std::atomic<uint64_t> s_CoId {0};
static std::atomic<uint64_t> s_CoNum {0};

// t_co is the coroutine currently executing on this thread
static thread_local Coroutine* t_co = nullptr;
static thread_local Coroutine::ptr t_main_thread_co = nullptr;  // 这个是调度协程

static size_t g_coStackSize = Config::Instance().getSize("coroutine", "stackSize", 1024 * 1024);
// static size_t coPoolSize


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

// Create a Coroutine for the currently executing thread
// No other Coroutine object represents the currently executing thread
// state() == EXEC
Coroutine::Coroutine() {
    // coId_ = ++s_CoId;
    state_ = State::EXEC;
    SetThis(this);
    ++s_CoNum;
    DEBUG("Coroutine:Coroutine sched co");
}

// 有参构造才是真正创建协程
Coroutine::Coroutine(const CallBack& cb, size_t stackSize, bool mainSched)
    : coId_(++s_CoId),
    callback_(cb) {
    ++s_CoNum;
    stackSize_ = static_cast<uint32_t>(stackSize? stackSize : g_coStackSize);
    // std::cout << "stackSize_" << stackSize_ << std::endl; // for test
    stack_ = StackAllocator::Alloc(stackSize_);

    if(!mainSched) {
        ctx_ = make_fcontext(static_cast<char*>(stack_) + stackSize_, stackSize_, &Coroutine::MainFunc);
    } else {
        ctx_ = make_fcontext(static_cast<char*>(stack_) + stackSize_, stackSize_, &Coroutine::CallMainFunc);
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

// 重置协程函数，并重置状态
void Coroutine::reset(CallBack cb) {
    BURGER_ASSERT(stack_);
    BURGER_ASSERT(state_ == State::TERM 
        || state_ == State::EXCEPT 
        || state_ == State::INIT);
    callback_ = cb;
    ctx_ = make_fcontext(static_cast<char*>(stack_) + stackSize_, stackSize_, &Coroutine::MainFunc);
    state_ = State::INIT;
}

// 当前协程切换到运行状态
void Coroutine::swapIn() {
    SetThis(this);  // 先将此协程设置成当前协程
    BURGER_ASSERT(state_ != State::EXEC); 
    state_ = State::EXEC;
    // jump_fcontext(&Scheduler::GetSchedCo()->ctx_, ctx_, 0);
    jump_fcontext(&t_main_thread_co->ctx_, ctx_, 0);
}

// 将当前线程切换到后台
void Coroutine::swapOut() {
    SetThis(Scheduler::GetSchedCo());
    jump_fcontext(&ctx_, t_main_thread_co->ctx_, 0);
    // jump_fcontext(&ctx_, Scheduler::GetSchedCo()->ctx_, 0);
}

// 当前线程切换到执行状态
// pre: 执行的为当前线程的主协程
void Coroutine::call() {
    SetThis(this);
    assert(state_ != State::EXEC);
    state_ = State::EXEC;
    jump_fcontext(&t_main_thread_co->ctx_, ctx_, 0);  // todo if check
}

// 当前线程切换到后台
// pre : 执行的为该协程
// 返回到现成的主协程
void Coroutine::back() {
    SetThis(t_main_thread_co.get());
    jump_fcontext(&ctx_, t_main_thread_co->ctx_, 0);
}

// 设置协程为运行协程
void Coroutine::SetThis(Coroutine* co) {
    t_co = co;
}

// 返回当前所在的协程
// 如果没有的话那么创建一个调度协程，由无参ctor构造
// 
Coroutine::ptr Coroutine::GetThis() {
    if(t_co) {
        return t_co->shared_from_this();
    }
    Coroutine::ptr schedCo = std::make_shared<Coroutine>();
    // 构造的时候就把当前t_co设置成为了schedCo
    BURGER_ASSERT(t_co == schedCo.get());
    t_main_thread_co = schedCo;  // 调度协程设置为这个第一次创建的
    return t_co->shared_from_this();
}

// 协程让出，设置为ready
void Coroutine::YieldToReady() {
    Coroutine::ptr cur = GetThis();
    BURGER_ASSERT(cur->state_ == State::EXEC);
    cur->state_ = State::READY;
    cur->swapOut();
}

void Coroutine::YieldToHold() {
    Coroutine::ptr cur = GetThis();
    BURGER_ASSERT(cur->state_ == State::EXEC);
    cur->state_ = State::HOLD;  // todo: race condition -- how to solve
    cur->swapOut();
}

uint64_t Coroutine::getCoNums() {
    return s_CoNum;
}

// 此两个函数可以优化

// 执行完成返回到线程主协程
void Coroutine::MainFunc(intptr_t vp) {
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

// 执行完成返回到线程调度协程
void Coroutine::CallMainFunc(intptr_t vp) {
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






 



