#include "burger/db/DB.h"

#include <map>
#include <string>
#include <iostream>

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
    MySQLRes::ptr res = mysql->query("select * from hams");
    res->foreach(dataCallback);

    MySQLStmt::ptr stmt = MySQLStmt::Create(mysql, "update hams SET name = ? where id = 1");
    stmt->bindString(1, "MIT-- SK");
    stmt->execute();
    std::cout << "--------- after execute -------" << std::endl;
    res = mysql->query("select * from hams");
    res->foreach(dataCallback);
    
}