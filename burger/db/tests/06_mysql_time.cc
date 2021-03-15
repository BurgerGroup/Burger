#include "burger/db/DB.h"

#include <iostream>

using namespace burger;
using namespace burger::db;

// apt-get install tzdata 换时区


int main() {
    MYSQL_TIME mt;
    time_t_to_mysql_time(time(0), mt);

    std::cout << mt.year << std::endl;
    std::cout << mt.month << std::endl;
    std::cout << mt.day << std::endl;
    std::cout << mt.hour << std::endl;
    std::cout << mt.minute << std::endl;
    std::cout << mt.second << std::endl;

    std::cout << "------------------" << std::endl;
    time_t t;
    mysql_time_to_time_t(mt, t);
    std::cout << t << std::endl;

}