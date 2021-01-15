#include "burger/base/Exception.h"
#include <iostream>

class Bar {
public:
    void test() {
        throw burger::Exception("oops");
    }
};

void foo() {
    Bar b;
    b.test();
}

int main() {
    try {
        foo();
    } catch (const burger::Exception& ex) {
        std::cout << "reasons: " << std::string(ex.what()) << std::endl;
        std::cout << "stack trace: " << std::string(ex.stackTrace()) << std::endl;
    }
}