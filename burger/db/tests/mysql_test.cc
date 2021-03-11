#include <iostream>
#include <string>
#include <map>

#include "burger/db/mysql.h"

using namespace burger;
using namespace burger::db;

int main() {
    std::map<std::string, std::string> params;
    params["host"] = "127.0.0.1";
    params["user"] = "burger";
    params["passwd"] = "123";
    params["dbname"] = "burger";
    MySQL::ptr mysql = std::make_shared<MySQL>(params);
    
}