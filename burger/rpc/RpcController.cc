#include "RpcController.h"

using namespace burger;
using namespace burger::rpc;

RpcController::RpcController() {
    failed_ = false;
    errText_ = "";
}

void RpcController::Reset() {
    failed_ = false;
    errText_ = "";
}

bool RpcController::Failed() const {
    return failed_;
}

std::string RpcController::ErrorText() const {
    return errText_;
}

void RpcController::SetFailed(const std::string& reason) {
    failed_ = true;
    errText_ = reason;
}








