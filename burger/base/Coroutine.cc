#include "Coroutine.h"
#include "Util.h"
#include "Log.h"
#include "Config.h"

using namespace burger;

static thread_local uint64_t s_coId {0};
static thread_local uint64_t s_coNum {0};

static size_t g_coStackSize = Config::Instance().getSize("coroutine", "stackSize", 1024*1024);

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

Coroutine::Coroutine(const Callback& cb, std::string name, size_t stackSize)
    : coId_(++s_coId),
    state_(State::EXEC),
    name_(name + "-" +std::to_string(coId_)),
    cb_(cb) {
    ++s_coNum;
    stackSize_ = stackSize? stackSize : g_coStackSize;
    stack_ = StackAllocator::Alloc(stackSize_);
    if(!stack_) {
        ERROR("malloc error");
    }
    ctx_ = make_fcontext(static_cast<char*>(stack_) + stackSize_, stackSize_, &Coroutine::RunInCo);
}

Coroutine::Coroutine()
    : coId_(++s_coId),
    state_(State::EXEC),
    name_("Main -" + std::to_string(coId_)) {
    ++s_coNum;
    DEBUG("MAIN Coroutine ctor, coId = {}", s_coId);
}

Coroutine::~Coroutine() {
    --s_coNum;
    if(stack_) {
        StackAllocator::Dealloc(stack_, stackSize_);
    }
    DEBUG("Coroutine::~Coroutine coId = {} total = {}", coId_, s_coNum);
}

// 挂起当前正在执行的协程，切换到主协程执行，必须在非主协程调用
void Coroutine::SwapOut() {
    assert(GetCurCo() != nullptr);
    // 必须在非主协程调用
    if (GetCurCo() == GetMainCo()) {
	return;
    }
    // 此处可以用智能指针否
    Coroutine* curCo = GetCurCo().get();
    GetCurCo() = GetMainCo();
    jump_fcontext(&curCo->ctx_, GetMainCo()->ctx_, 0);
}
// 挂起主协程，执行当前协程，只能在主协程调用
void Coroutine::swapIn() {
    if(state_ == State::TERM) return;
    GetCurCo() = shared_from_this();
    jump_fcontext(&GetMainCo()->ctx_, ctx_, 0);
}

uint64_t Coroutine::GetCoId() {
    assert(GetCurCo() != nullptr);
    return GetCurCo()->coId_;
}

Coroutine::ptr& Coroutine::GetCurCo() {
    //第一个协程对象调用swapIn()时初始化
    // todo : 写在外面或许更好?
    static thread_local Coroutine::ptr t_curCo;
    return t_curCo;
}

Coroutine::ptr Coroutine::GetMainCo() {
    static thread_local Coroutine::ptr t_main_co = std::make_shared<Coroutine>();
    return t_main_co;
}

void Coroutine::RunInCo(intptr_t vp) {
    Coroutine::ptr cur = GetCurCo();
    cur->cb_();
    cur->cb_ = nullptr;
    cur->setState(State::TERM);
    Coroutine::SwapOut();   	//重新返回主协程
}

