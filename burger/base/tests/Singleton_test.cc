#include "burger/base/Singleton.h"
#include <boost/noncopyable.hpp>
#include <string>
#include <thread>
#include <iostream>

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

void ThreadFunc() {
    std::cout << "tid = " << std::this_thread::get_id()
        << " Address = " << static_cast<const void *>(&burger::Singleton<Test>::Instance())
        << " name = " << burger::Singleton<Test>::Instance().getName() << std::endl;
    burger::Singleton<Test>::Instance().setName("only one, changed");
}

int main() {
    burger::Singleton<Test>::Instance().setName("only one");
    std::thread t1(ThreadFunc);
    t1.join();
    std::cout << "tid = " << std::this_thread::get_id()
        << " Address = " << static_cast<const void *>(&burger::Singleton<Test>::Instance())
        << " name = " << burger::Singleton<Test>::Instance().getName() << std::endl;
}