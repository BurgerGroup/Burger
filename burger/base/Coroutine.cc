#include "Coroutine.h"
#include "Util.h"
#include "Log.h"
#include "Config.h"

using namespace burger;

static thread_local Coroutine* t_co = nullptr;
static thread_local Coroutine::ptr t_main_co = nullptr;

static std::atomic<uint64_t> s_coId {0};
static std::atomic<uint64_t> s_coNum {0};

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
    assert(stackSize_ > 0);
    stack_ = StackAllocator::Alloc(stackSize_);
    if(!stack_) {
        ERROR("malloc error");
    }
    ctx_ = make_fcontext(static_cast<char*>(stack_) + stackSize_, stackSize_, &Coroutine::RunInCo);
    DEBUG("Coroutine ctor, coId = {}", s_coId);
}

Coroutine::Coroutine()
    : coId_(++s_coId),
    state_(State::EXEC),
    name_("Main -" + std::to_string(coId_)) {
    ++s_coNum;
    SetThisCo(this);
    DEBUG("MAIN Coroutine ctor, coId = {}", s_coId);
}

Coroutine::~Coroutine() {
    --s_coNum;
    if(stack_) {
        BURGER_ASSERT(state_ == State::TERM);
        StackAllocator::Dealloc(stack_, stackSize_);
    } else {
        BURGER_ASSERT(!cb_);
        BURGER_ASSERT(state_ == State::EXEC);
        Coroutine* cur = t_co;
        if(cur == this) {
            SetThisCo(nullptr);
        }
    }
    DEBUG("Coroutine::~Coroutine coId = {} total = {}", coId_, s_coNum);
}

// 挂起当前正在执行的协程，切换到主协程执行，必须在非主协程调用
void Coroutine::SwapOut() {
    // 必须在非主协程调用
    if (GetCurCo() == t_main_co) {
	    return;
    }
    // jump 切换不回来，如果此处用智能指针将无法释放
    // Coroutine* curCo = GetCurCo().get();
    Coroutine* curCo = t_co;
    SetThisCo(t_main_co.get());
    jump_fcontext(&curCo->ctx_, t_main_co->ctx_, 0);
}
// 挂起主协程，执行当前协程，只能在主协程调用
void Coroutine::swapIn() {
    SetThisCo(this);
    if(state_ == State::TERM) return;
    jump_fcontext(&GetMainCo()->ctx_, ctx_, 0);
}

uint64_t Coroutine::GetCoId() {
    if(t_co) {
        return t_co->getCoId();
    }
    return 0;
}

Coroutine::ptr Coroutine::GetCurCo() {
    if(t_co) {
        return t_co->shared_from_this();
    }
    // 当前无co的时候，创建一个无参的主协程
    Coroutine::ptr mainCo = std::make_shared<Coroutine>();
    assert(t_co == mainCo.get());
    t_main_co = mainCo;
    return t_co->shared_from_this();
}

Coroutine::ptr Coroutine::GetMainCo() {
    if(t_main_co) {
        return t_main_co;
    }
    Coroutine::ptr mainCo = std::make_shared<Coroutine>();
    t_main_co = mainCo;
    return t_main_co;
}

void Coroutine::SetThisCo(Coroutine* co) {
    t_co = co;
}

void Coroutine::RunInCo(intptr_t vp) {
    Coroutine::ptr cur = GetCurCo();
    cur->cb_();
    cur->cb_ = nullptr;
    cur->setState(State::TERM);
    cur.reset();  // 防止无法析构
    Coroutine::SwapOut();   	//重新返回主协程
}
