#include <iostream>
#include <string>
#include <map>
#include "burger/db/mysql.h"
// #include "Burger/db/MysqlRes.h"
/*
Create a connection
Execute query
Get the result set
Fetch all available rows
Free the result set
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
    // dynamic_pointer_cast 当指针是智能指针时候，向下转换
    MySQLRes::ptr res = mysql->query("select * from hams");
    std::cout << "rows : "<< res->getRowCount() << std::endl;
    std::cout << "columns : " << res->getColumnCount() << std::endl;

    

}