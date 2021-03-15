#include <iostream>
#include <string>
#include <map>
#include "burger/db/DB.h"

/*
- retrieve data

Create a connection
Execute query
Get the result set
Fetch all available rows

*/
using namespace burger;
using namespace burger::db;

bool dataCallback(MYSQL_ROW row
                ,uint64_t columnNum, int row_no) {
    if(!row || columnNum <= 0) return false;
    std::cout << std::endl;
    std::cout << "row " << row_no << std::endl;
    for(uint64_t i = 0; i < columnNum; i++) {
        std::cout << (row[i] ?  row[i] : "NULL") << " ";
    }
    
    return true;
}

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

    res->foreach(dataCallback);
    

}