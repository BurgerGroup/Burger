#ifndef B_H
#define B_H

#include <functional>

class B {
public:
    using CallBack = std::function<void()>;
    // using CallBack = std::function<void(int)>;
    B() = default;
    ~B() = default;
    void setCallback(CallBack cb) { cb_ = std::move(cb);  }
    void execute() { cb_(); }
private:
    CallBack cb_;
};

#endif // B_H