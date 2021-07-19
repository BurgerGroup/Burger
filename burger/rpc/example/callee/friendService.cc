#include <iostream>
#include <string>
#include <vector>
#include "friend.pb.h"   // 如何优雅的不要相对路径
#include "burger/rpc/RpcServer.h"
#include "burger/base/Log.h"

using namespace burger; 
using namespace burger::rpc;

class FriendService : public burgerRpc::FriendServiceRpc {
public:    
    std::vector<std::string> GetFriendList(uint32_t userid) {
        INFO("do GetFriendList service! userid : {}", userid);
        std::vector<std::string> vec;
        vec.push_back("MIT SK");
        vec.push_back("Ligh");
        vec.push_back("aze");
        return vec;
    }
    // 重写基类方法
    void GetFriendList(::google::protobuf::RpcController* controller,
                       const ::burgerRpc::GetFriendListRequest* request,
                       ::burgerRpc::GetFriendListResponse* response,
                       ::google::protobuf::Closure* done) {
        // 取出数据
        uint32_t userid = request->userid();
        std::vector<std::string> friendsList = GetFriendList(userid);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for(std::string& name : friendsList) {
            std::string *p = response->add_friends();
            *p = name;
        }
        done->Run();
    }
};

int main(int argc, char **argv) {
    LOGGER(); LOG_LEVEL_INFO;
    RpcServer provider;
    provider.NotifyService(new FriendService());
    provider.Run();
    return 0;
}