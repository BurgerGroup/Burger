#ifndef RPCCONTROLLER_H
#define RPCCONTROLLER_H

#include <google/protobuf/service.h>
#include <string>

class RpcController : public google::protobuf::RpcController {
public:
    RpcController();
    void Reset();
    
private:  

};


#endif // RPCCONTROLLER_H