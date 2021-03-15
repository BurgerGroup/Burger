#include <iostream>
#include <string>
#include <map>
#include "burger/db/DB.h"

/*
- retrieve data

test next
*/
using namespace burger;
using namespace burger::db;

int main() {
    std::map<std::string, std::string> params;
    params["host"] = "127.0.0.1";
    params["user"] = "root";
    params["passwd"] = "PWD";
    params["dbname"] = "burger";
    MySQL::ptr mysql = std::make_shared<MySQL>(params);
    mysql->connect();
    MySQLRes::ptr res = mysql->query("select * from hams");

    int i = 0;
    while(res->next()) {
        std::cout << std::endl;
        std::cout << "row : " << i++ << " : ";
        std::cout << res->getInt(0) << " - " << res->getString(1);
    }
    

}