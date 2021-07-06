#include "RpcApp.h"
#include "Config.h"

using namespace burger;
using namespace burger::rpc;

// 通过配置文件读入参数
void RpcApp::Init() {
    std::string rpcServerIp = Config::Instance().getString("rpc", "rpcServerIp", "127.0.0.1");
    std::string rpcServerPort = Config::Instance().getString("rpc", "rpcServerPort", "8000");
    std::string zookeeperIp = Config::Instance().getString("rpc", "zookeeperIp", "127.0.0.1");
    std::string zookeeperPort = Config::Instance().getString("rpc", "zookeeperPort", "5000");
}
