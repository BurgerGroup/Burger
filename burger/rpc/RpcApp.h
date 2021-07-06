#ifndef RPCAPP_H
#define RPCAPP_H

namespace burger {
namespace rpc {

// 框架的基础类, 负责框架的一些初始化操作
// 单例

class RpcApp {
public:  
    static void Init();
    static RpcApp& GetInstance() {
        static RpcApp app;
        return app;
    }
private:  
    RpcApp() = default;
    RpcApp(const RpcApp&) = delete;
    RpcApp(RpcApp&&) = delete;
};

} // namespace rpc
} // namespace burger





#endif // RPCAPP_H