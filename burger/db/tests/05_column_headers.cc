#include <iostream>
#include <string>
#include <map>
#include "burger/db/DB.h"

using namespace burger;
using namespace burger::db;
/*
mysql> select * from hams limit 3;
+----+----------+-------+
| id | name     | price |
+----+----------+-------+
|  1 | Audi     | 52642 |
|  2 | Mercedes | 57127 |
|  3 | Skoda    |  9000 |
+----+----------+-------+

The MYSQL_FIELD structure contains information about a field, such as the field's name, type and size.
 Field values are not part of this structure; they are contained in the MYSQL_ROW structure

*/

int main() {
    std::map<std::string, std::string> params;
    params["host"] = "127.0.0.1";
    params["user"] = "root";
    params["passwd"] = "PWD";
    params["dbname"] = "burger";
    MySQL::ptr mysql = std::make_shared<MySQL>(params);
    mysql->connect();

    MySQLRes::ptr res = mysql->query("select * from hams LIMIT 3");
    int columnNum = res->getColumnCount();
    for(int i = 0; i < columnNum; i++) {
        std::cout << i << " " << res->getColumnName(i) << std::endl;
    }

    
}