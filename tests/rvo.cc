/*
https://www.cnblogs.com/kekec/p/11303391.html
https://www.zhihu.com/question/27000013

release 并没有拷贝构造函数
*/

#include <iostream>

class Foo {
public:
    Foo() { std::cout << "Foo ctor" << std::endl; }
    Foo(const Foo&) { std::cout << "Foo copy ctor" << std::endl; }
    Foo& operator=(const Foo&) { std::cout << "Foo operator= " << std::endl; }
    ~Foo() { std::cout << "Foo dtor" << std::endl; }
};
 
Foo make_foo() {
    Foo f;
    return f;
    // return Foo();
}

int main() {
    make_foo();
    return 0;
}