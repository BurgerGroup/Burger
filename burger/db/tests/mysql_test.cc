#include <iostream>
#include <string>
#include <map>

#include "burger/db/mysql.h"

using namespace burger;
using namespace burger::db;

int main() {
    std::map<std::string, std::string> params;
    params["host"] = "127.0.0.1";
    params["user"] = "root";
    params["passwd"] = "PWD";
    params["dbname"] = "burger";
    MySQL::ptr mysql = std::make_shared<MySQL>(params);
    
    if(!mysql->connect()) {
        std::cout << "connect fail" << std::endl;
        return -1;
    } else {
        std::cout << "connected... " << std::endl;
        mysql->mysqlInfo(); 
    }
}