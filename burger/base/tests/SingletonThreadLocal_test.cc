#include <boost/noncopyable.hpp>
#include "burger/base/Singleton.h"
#include <thread>
#include <iostream>
#include <string>

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

using ST = burger::SingletonPerThread<Test>;

void print() {
    std::cout << std::this_thread::get_id() << "  obj1 Address = "
        <<  static_cast<const void *>(&ST::Instance()) << "  name = " << ST::Instance().getName() <<std::endl;

}

void ThreadFunc(const std::string changeTo) {
    print();
    ST::Instance().setName(changeTo);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    print();
}

int main() {
    ST::Instance().setName("main one");
    std::thread t1(ThreadFunc, "thread1");
    std::thread t2(ThreadFunc, "thread2");
    t1.join();
    print();
    t2.join();
}