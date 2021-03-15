#include <iostream>
#include <string>
#include <map>
#include "burger/db/DB.h"

/*
Sometimes, we need to determine the id of the last inserted row. 
We can determine the last inserted row id by calling the mysql_insert_id() function. 
The function only works if we have defined an AUTO_INCREMENT column in the table.
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

    mysql->execute("DROP TABLE IF EXISTS burbur");
    mysql->execute("CREATE TABLE burbur(id INT PRIMARY KEY AUTO_INCREMENT, name VARCHAR(255))");
    mysql->execute("INSERT INTO burbur(name) VALUES('beef')");
    mysql->execute("INSERT INTO burbur(name) VALUES('pork')");
    //  returns the value generated for an AUTO_INCREMENT column by the previous INSERT or UPDATE statement.
    // todo : need more tests
    std::cout << "my last insert id = " << mysql->getLastInsertId();
    
}