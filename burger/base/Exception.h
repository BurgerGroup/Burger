#ifndef EXCEPTION_H
#define EXCEPTION_H
#include <exception>
#include <string>
#include <cxxabi.h>
#include <execinfo.h>
namespace burger {

class Exception : public std::exception {
public:
    explicit Exception(std::string msg);
    ~Exception() noexcept override = default; 
    const char* what() const noexcept override;
    const char* stackTrace() const noexcept;

private:
    std::string stackTrace(bool demangle);

private:
    std::string message_;
    std::string stack_;
};
} // namespace burger




#endif // EXCEPTION_H