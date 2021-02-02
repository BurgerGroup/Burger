#ifndef B_H
#define B_H

#include "A.h"
#include <functional>

class B {
public:
    using Callback = std::function<void()>;
    B() = default;
    ~B() = default;
    void setCallback(const Callback& cb) { cb_ = cb; } 
    void start() { 
        setCallback(std::bind(&A::print, std::ref(a_)));
        if(cb_) cb_();
    }
private:
    Callback cb_;
    std::unique_ptr<A> a_;
};

#endif // B_H