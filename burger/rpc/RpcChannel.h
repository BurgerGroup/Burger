#ifndef RPCCHANNEL_H
#define RPCCHANNEL_H

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <string>

namespace burger {
namespace rpc {

class RpcChannel : public ::google::protobuf::RpcChannel {
public:
    // 所有通过stub代理对象调用的rpc方法，都走到这里，统一做rpc方法调用的数据序列化和网络发送
    void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                ::google::protobuf::RpcController* controller,
                const ::google::protobuf::Message* request,
                ::google::protobuf::Message* response,
                ::google::protobuf::Closure* done) override;
};



} // namespace rpc

} // namespace burger



#endif // RPCCHANNEL_H