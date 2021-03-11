#ifndef MYSQLTIME_H
#define MYSQLTIME_H
#include <mysql/mysql.h>

#include <ctime>

namespace burger {
namespace db {
    
struct MySQLTime {
    MySQLTime(time_t t)
        :ts(t) { }
    time_t ts;
};

bool mysql_time_to_time_t(const MYSQL_TIME& mt, time_t& ts);
bool time_t_to_mysql_time(const time_t& ts, MYSQL_TIME& mt);




} // namespace db

} // namespace burger



#endif // MYSQLTIME_H