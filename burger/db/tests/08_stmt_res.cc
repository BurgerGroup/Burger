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

    mysql->execute("INSERT INTO hams VALUES(12,'SKSKMIT',1000)");
    mysql->execute("INSERT INTO hams VALUES(8,'LigHao',2000)");
    mysql->execute("INSERT INTO hams VALUES(9,'Gaib',1000)");
    mysql->execute("INSERT INTO hams VALUES(10,'Dr Zhan',3000)");
    mysql->execute("INSERT INTO hams VALUES(11,'Hands',1000)");

    MySQLRes::ptr res = mysql->query("select * from hams");
    res->foreach(dataCallback);   

    auto stmt = mysql->prepare("select * from hams where id >= ?");
    stmt->bind(1, 8);
    auto stmtres = stmt->query();
    stmtres->getDataCount();



}