#ifndef RPCCONTROLLER_H
#define RPCCONTROLLER_H

#include <google/protobuf/service.h>
#include <string>

namespace burger {
namespace rpc {

class RpcController : public google::protobuf::RpcController {
public:
    RpcController();
    void Reset();
    bool Failed() const;
    std::string ErrorText() const;
    void SetFailed(const std::string& reason);
    // 目前未实现的具体功能
    void StartCancel() {};
    bool IsCanceled() const { return false; };
    void NotifyOnCancel(google::protobuf::Closure* callback) {};
private:  
    bool failed_;  // RPC 方法执行过程中的状态
    std::string errText_;   // RPC 方法执行过程中的错误信息
};

} // namespace rpc
} // namespace burger

#endif // RPCCONTROLLER_H