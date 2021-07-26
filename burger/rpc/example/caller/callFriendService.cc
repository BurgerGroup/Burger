#include <iostream>
#include "friend.pb.h"
#include "burger/rpc/RpcChannel.h"
#include "burger/rpc/RpcController.h"
#include "burger/base/Log.h"

using namespace burger;
using namespace burger::rpc;

int main() {
    RpcChannel rpcChannel;
    burgerRpc::FriendServiceRpc_Stub stub(&rpcChannel);  

    burgerRpc::GetFriendListRequest request;
    request.set_userid(1999);

    burgerRpc::GetFriendListResponse response;

    RpcController controller;

    stub.GetFriendList(nullptr, &request, &response, nullptr);

    if(controller.Failed()) {
        ERROR("{}", controller.ErrorText());
    } else {
        if(0 == response.result().errcode()) {
            INFO("RPC GetFriendList response success");
            int size = response.friends_size();
            for(int i = 0; i < size; i++) {
                INFO("index : {} - name : {}", i+1, response.friends(i));
            }
        } else {
            ERROR("RPC GetFriendList response error : {}", response.result().errmsg());
        }
    }


    return 0;
}