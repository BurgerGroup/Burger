#include <functional>
#include <iostream>
#include <string>

class A {
public:
    A() {
        std::cout << "construct" << std::endl;
    }

    A(const A& a) {
        member = a.member;
        std::cout << "copy construct" << std::endl;
    }

    A(A&& a) {
        member = std::move(a.member);
        std::cout << "rvalue copy construct" << std::endl;
    }

    ~A() {
        std::cout << "destruct" << std::endl;
    }

public:
    std::string member;
};


void argumentObjectPassByValue(A a) {
    std::cout << "pass by value" << std::endl;
}

void argumentObjectPassByRef(const A &a) {
    std::cout << "pass by const ref" << std::endl;
}

void func1() {
    // construct
    // copy construct
    // pass by value
    // destruct
    // destruct
    A a;
    argumentObjectPassByValue(a);
}

void func2() {
    // construct
    // pass by const ref
    // destruct
    A a;
    argumentObjectPassByRef(a);
}

void func3() {
    // construct
    // copy construct
    // pass by value
    // destruct
    // destruct
    A a;
    auto f = std::bind(argumentObjectPassByValue, std::placeholders::_1);
    f(a);
}

void func4() {
    // construct
    // pass by const ref
    // destruct
    A a;
    auto f = std::bind(argumentObjectPassByRef, std::placeholders::_1);
    f(a);
}

void func5() {
    // construct
    // copy construct
    // rvalue copy construct
    // pass by value
    // destruct
    // destruct
    // destruct
    A a;
    using callback = std::function<void(A)>;
    callback f = std::bind(argumentObjectPassByValue, std::placeholders::_1);
    f(a);
}

void func6() {
    // construct
    // pass by const ref
    // destruct
    A a;
    using callback = std::function<void(const A&)>;
    callback f = std::bind(argumentObjectPassByRef, std::placeholders::_1);
    f(a);
}

void func7() {
    // construct
    // copy construct
    // copy construct
    // pass by value
    // destruct
    // destruct
    // destruct
    A a;
    auto f = std::bind(argumentObjectPassByValue, a);
    f();
}

void func8() {
    // construct
    // copy construct
    // pass by const ref
    // destruct
    // destruct
    A a;
    auto f = std::bind(argumentObjectPassByRef, a);
    f();
}


void func9() {
    // construct
    // copy construct
    // pass by value
    // destruct
    // destruct
    A a;
    auto f = std::bind(argumentObjectPassByValue, std::ref(a));
    f();
}

void func10() {
    // construct
    // pass by const ref
    // destruct
    A a;
    auto f = std::bind(argumentObjectPassByRef, std::ref(a));
    f();
}

void func11() {
    // construct
    // copy construct
    // rvalue copy construct
    // destruct
    // copy construct
    // pass by value
    // destruct
    // destruct
    // destruct
    A a;
    using callback = std::function<void()>;
    callback f = std::bind(argumentObjectPassByValue, a);
    f();
}

void func12() {
    // construct
    // copy construct
    // rvalue copy construct
    // destruct
    // pass by const ref
    // destruct
    // destruct
    A a;
    using callback = std::function<void()>;
    callback f = std::bind(argumentObjectPassByRef, a);
    f();
}


void func13() {
    // construct
    // copy construct
    // pass by value
    // destruct
    // destruct
    A a;
    using callback = std::function<void()>;
    callback f = std::bind(argumentObjectPassByValue, std::ref(a));
    f();
}

void func14() {
    // construct
    // pass by const ref
    // destruct
    A a;
    using callback = std::function<void()>;
    callback f = std::bind(argumentObjectPassByRef, std::ref(a));
    f();
}




int main() {
    // func1();
    // func2();
    // func3();
    // func4();
    // func5();
    // func6();
    // func7();
    // func8();
    // func9();
    // func10();
    // func11();
    // func12();
    // func13();
    func14();

    
}