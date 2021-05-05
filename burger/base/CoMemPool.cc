#include "CoMemPool.h"
#include <stdlib.h>

using namespace burger;

CoMemPool::CoMemPool(size_t maxSize) 
    : maxSize_(maxSize) {
}

CoMemPool::~CoMemPool() {
    for(const auto& mem : mems_) {
        free(mem);
    }
}

void* CoMemPool::Alloc(size_t size) {
    for(auto it = mems_.begin(); it != mems_.end(); it++) {
        if((*it)->size >= size) {
            auto p  = (*it)->data;
            mems_.erase(it);
            return p;
        }
    }
    MemItem* item = static_cast<MemItem*>(malloc(sizeof(size_t) + size));
    item->size = size;
    return item->data;
}

void CoMemPool::Dealloc(void *ptr) {
    // todo : ???
    MemItem* item = reinterpret_cast<MemItem*>(static_cast<char*>(ptr) - sizeof(size_t));
    mems_.push_front(item); // todo: why push_front here
    if(mems_.size() > maxSize_) {
        MemItem* item1 = mems_.back();
        mems_.pop_back();
        free(item1);
    }
}






