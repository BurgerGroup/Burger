#include "json/json.hpp"

using json = nlohmann::json;

#include <iostream>
#include <vector>
#include <map>
#include <string>
using namespace std;

string func1() {
    json js;
    js["msg_type"] = 2;
    js["from"] = "sgwf";
    js["to"] = "ysy";
    js["msg"] = "hello, what are you doing now?";
    
    cout << js << endl;
    string sendBuf = js.dump();
    cout << sendBuf <<endl;  
    return sendBuf;
}

string func2() {
    json js;
    // 添加数组
    js["id"] = {1, 2, 3, 4, 5};
    // 添加key-value
    js["name"] = "sgwf";
    // 添加对象
    js["msg"]["sgwf"] = "dazhuzhu";
    js["msg"]["ysy"] = "say hi to dazhuzhu";
    // 上面等同于下面这句一次性添加数组对象
    // 覆盖了，key不允许覆盖
    js["msg"] = {{"sgwf", "hello world"}, {"ysy", "hello china"}};
    cout << js << endl;
    return js.dump();
}

// json可以序列化容器
string func3() {
    json js;

    // 直接序列化一个vector容器
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(5);

    js["list"] = vec;

    // 直接序列化一个map容器
    map<int, string> m;
    m.insert({1, "MIT!!"});
    m.insert({2, "啸哥"});
    m.insert({3, "学活了"});

    js["path"] = m;

    string sendBuf = js.dump(); // json数据对象 =》序列化 json字符串
    cout << sendBuf << endl;
    return sendBuf;
}

void func4(const string& recvBuf) {
    // 数据的反序列化   json字符串 => 反序列化 数据对象（看作容器，方便访问）
    json jsbuf = json::parse(recvBuf);
    cout << jsbuf["msg_type"] << endl;
    cout << jsbuf["from"] << endl;
    cout << jsbuf["to"] << endl;
    cout << jsbuf["msg"] << endl;

}

int main() {
    // string recvBuf = func1();
    // func4(recvBuf);

    // func2();
    func3();
    // cout<<jsbuf["id"]<<endl;
    // auto arr = jsbuf["id"];
    // cout<<arr[2]<<endl;

    // auto msgjs = jsbuf["msg"];
    // cout<<msgjs["zhang san"]<<endl;
    // cout<<msgjs["liu shuo"]<<endl;

    // vector<int> vec = jsbuf["list"]; // js对象里面的数组类型，直接放入vector容器当中
    // for (int &v : vec)
    // {
    //     cout << v << " ";
    // }
    // cout << endl;

    // map<int, string> mymap = jsbuf["path"];
    // for (auto &p : mymap)
    // {
    //     cout << p.first << " " << p.second << endl;
    // }
    // cout << endl;

    return 0;
}