#include "burger/base/Atomic.h"
#include <cassert>
#include <iostream>

int main()
{
    {
        burger::AtomicInt64 a0;
        assert(a0.get() == 0);
        assert(a0.getAndAdd(1) == 0);
        assert(a0.get() == 1);
        assert(a0.addAndGet(2) == 3);
        assert(a0.get() == 3);
        assert(a0.incrementAndGet() == 4);
        assert(a0.get() == 4);
        a0.increment();
        assert(a0.get() == 5);
        assert(a0.addAndGet(-3) == 2);
        assert(a0.getAndSet(100) == 2);
        assert(a0.get() == 100);
    }

    {
        burger::AtomicInt32 a1;
        assert(a1.get() == 0);
        assert(a1.getAndAdd(1) == 0);
        assert(a1.get() == 1);
        assert(a1.addAndGet(2) == 3);
        assert(a1.get() == 3);
        assert(a1.incrementAndGet() == 4);
        assert(a1.get() == 4);
        a1.increment();
        assert(a1.get() == 5);
        assert(a1.addAndGet(-3) == 2);
        assert(a1.getAndSet(100) == 2);
        assert(a1.get() == 100);
    }
    {
        burger::AtomicInt32 a2;
        if(a2.get()) {
            std::cout << "a2 is true" << std::endl; 
        } else {
            std::cout << "a2 is false" << std::endl;
        }
        a2.increment();
        if(a2.get()) {
            std::cout << "a2 increment is true" << std::endl; 
        } else {
            std::cout << "a2 increment is false" << std::endl;
        }
    }
}