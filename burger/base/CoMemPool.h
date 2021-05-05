#ifndef COMEMPOOL_H
#define COMEMPOOL_H

#include <list>
#include <functional>


// todo : co 内存池

class CoMemPool {
friend class Coroutine;
public: 
    using Callback = std::function<void()>;
    CoMemPool(size_t maxSize);
    ~CoMemPool();
    void *Alloc(size_t size);
    void Dealloc(void *ptr);
    int getSize() { return mems_.size(); }
    static Coroutine* NewCo(const Callback& cb, size_t stackSize);

private:
    struct MemItem {
        size_t size;
        char data[];
    }
    size_t maxSize_;
    std::list<MemItem*> mems_;


};


#endif // COMEMPOOL_H