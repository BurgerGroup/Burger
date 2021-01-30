#ifndef A_H
#define A_H
#include "B.h"
#include <memory>
#include <iostream>

class A {
public:
    A();
    ~A() { b_->execute(); }
private:
    void handlecb();
    std::unique_ptr<B> b_;
};


#endif // A_H