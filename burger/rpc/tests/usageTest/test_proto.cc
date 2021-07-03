#include "test.pb.h"
#include <iostream>

using namespace burgerProto;

void test01() {
    // 封装了login请求对象的数据
    LoginRequest req;
    req.set_name("ligh");
    req.set_pwd("123");
    std::string send_str;
    // 对象序列化
    if(req.SerializeToString(&send_str)) {
        std::cout << send_str << std::endl;
    }
    LoginRequest reqB;
    // 反序列化
    if(reqB.ParseFromString(send_str)) {
        std::cout << reqB.name() << std::endl;
        std::cout << reqB.pwd() << std::endl;
    }
}

void test02() {
    addFriendResponse rsp;
    // 这里是返回了指针
    ResultCode* rc = rsp.mutable_result();
    rc->set_errcode(0);
    rc->set_errmsg("add Successfully");
}

void test03() {
    GetFriendListResponse rsp;
    ResultCode* rc = rsp.mutable_result();
    rc->set_errcode(0);

    User* user1 = rsp.add_friendlist();
    user1->set_name("ligh");
    user1->set_age(20);
    user1->set_sex(User::MAN);

    User* user2 = rsp.add_friendlist();
    user2->set_name("ligh");
    user2->set_age(20);
    user2->set_sex(User::MAN);

    std::cout << rsp.friendlist_size() << std::endl;
}

int main() {
    // test01();
    // test02();
    test03();
    return 0;
}
