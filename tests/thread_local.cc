#include <thread>
#include <string>
#include <boost/noncopyable.hpp>
#include <iostream>

// 每个线程里面的地址不一样了

class Test :boost::noncopyable {
public:
    Test() { 
        std::cout << "Test tid = " << std::this_thread::get_id() << "  Address = "
            <<  static_cast<const void *>(this) << std::endl;
    }
    
    ~Test() {
        std::cout << "~Test tid = " << std::this_thread::get_id() << "  Address = "
        <<  static_cast<const void *>(this) << "  name = " << name_ <<std::endl;
    }
    const std::string& getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

private:
    std::string name_;
};

thread_local Test testObj1;
thread_local Test testObj2;

void print() {
    std::cout << std::this_thread::get_id() << "  obj1 Address = "
        <<  static_cast<const void *>(&testObj1) << "  name = " << testObj1.getName() <<std::endl;
    std::cout << std::this_thread::get_id() << "  obj2 Address = "
        <<  static_cast<const void *>(&testObj2) << "  name = " << testObj2.getName() <<std::endl;
}

void ThreadFunc() {
    print();
    testObj1.setName("changed 1");
    testObj2.setName("changed 42");
    print();
}

int main() {
    testObj1.setName("main one");
    print();
    std::thread t1(ThreadFunc);
    t1.join();
    testObj2.setName("main two");
    print();
    
}