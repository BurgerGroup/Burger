#include "Exception.h"

using namespace burger;


namespace burger {

Exception::Exception(std::string msg)
  : message_(std::move(msg)),
    stack_(CurrentThread::stackTrace(/*demangle=*/false))
{}

const char* Exception::what() const noexcept {
    return message_.c_str();
}

const char* Exception::stackTrace() const noexcept {
    return stack_.c_str();
}


}