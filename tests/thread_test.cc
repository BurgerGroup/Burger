#include <thread>
#include <iostream>
#include <string>
#include <unistd.h>
void threadFunc() {
    std::cout << "tid : " << std::this_thread::get_id() << std::endl;
}

void threadFunc2(int x) {
    std::cout << "tid : " << std::this_thread::get_id()
                << " x = " << x << std::endl;
}

class Foo {
public:
    explicit Foo(double x) : x_(x) 
    {}
    void memFunc() {
        std::cout << "tid : " << std::this_thread::get_id()
                    << " x_ = " << x_ << std::endl;
    }

    void memFunc2(const std::string& text) {
        std::cout << "tid : " << std::this_thread::get_id()
                    << " text = " << text << std::endl;
    }
private:
    double x_;
};

int main() {
    std::cout << "Main thread : pid = " << ::getpid() << "\t tid = "
        << std::this_thread::get_id() << std::endl;
    std::thread t1(threadFunc);

    std::thread t2(threadFunc2, 42);

    Foo foo(87.53);
    std::thread t3(&Foo::memFunc, &foo);
    
    std::thread t4(&Foo::memFunc2, &foo, "burger");
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    
    // lack name 
    // lack numCreated
}