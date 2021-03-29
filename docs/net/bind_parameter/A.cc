#include "A.h"


A::A() : b_(std::make_unique<B>()) {
    b_->setCallback(std::bind(&A::handlecb, this));
}

void A::handlecb() {
    std::cout << "handlecb" << std::endl;
}
