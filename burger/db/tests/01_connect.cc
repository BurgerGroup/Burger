#include <iostream>
#include <string>
#include <map>
#include "burger/db/DB.h"

/*
connect 
create and populate table

mysql> show databases;
+--------------------+
| Database           |
+--------------------+
| information_schema |
| burger             |
| mysql              |
| performance_schema |
| sys                |
+--------------------+

mysql> use burger;
Database changed
mysql> show tables;
+------------------+
| Tables_in_burger |
+------------------+
| hams             |
+------------------+

mysql> select * from hams;
+----+----------+-------+
| id | name     | price |
+----+----------+-------+
|  1 | Audi     | 52642 |
|  2 | Mercedes | 57127 |
|  3 | Skoda    |  9000 |
+----+----------+-------+

*/
using namespace burger;
using namespace burger::db;

int main() {
    std::map<std::string, std::string> params;
    params["host"] = "127.0.0.1";
    params["user"] = "root";
    params["passwd"] = "PWD";
    // params["dbname"] = "burger";
    MySQL::ptr mysql = std::make_shared<MySQL>(params);
    
    mysql->connect();
    mysql->mysqlInfo(); 

    if(mysql->ping()) {
        std::cout << "Working ..." << std::endl;
    } else {
        std::cout << "Disconnected" << std::endl;
    }
    if(mysql->use("burger")) {
        std::cout << "use database successfully" << std::endl;
    } else {
        std::cout << "use database failed" << std::endl;
    }
    mysql->execute("DROP TABLE IF EXISTS hams");
    mysql->execute("CREATE TABLE hams(id INT PRIMARY KEY AUTO_INCREMENT, name VARCHAR(255), price INT)");
    mysql->execute("INSERT INTO hams VALUES(1,'Audi',52642)");
    mysql->execute("INSERT INTO hams VALUES(2,'Mercedes',57127)");
    mysql->execute("INSERT INTO hams VALUES(3,'Skoda',9000)");
    mysql->execute("INSERT INTO hams VALUES(4,'MITSK',92000)");
    mysql->execute("INSERT INTO hams VALUES(5,'BurgerY',900)");
}