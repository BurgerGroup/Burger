#ifndef MYSQLUTIL_H
#define MYSQLUTIL_H
#include "DB.h"

namespace burger {
namespace db {

class MySQLUtil {
public:
    static ISQLData::ptr Query(const std::string& name, const char* format, ...);
    static ISQLData::ptr Query(const std::string& name, const char* format,va_list ap); 
    static ISQLData::ptr Query(const std::string& name, const std::string& sql);

    static ISQLData::ptr TryQuery(const std::string& name, uint32_t count, const char* format, ...);
    static ISQLData::ptr TryQuery(const std::string& name, uint32_t count, const std::string& sql);

    static int Execute(const std::string& name, const char* format, ...);
    static int Execute(const std::string& name, const char* format, va_list ap); 
    static int Execute(const std::string& name, const std::string& sql);

    static int TryExecute(const std::string& name, uint32_t count, const char* format, ...);
    static int TryExecute(const std::string& name, uint32_t count, const char* format, va_list ap); 
    static int TryExecute(const std::string& name, uint32_t count, const std::string& sql);
};
    
} // namespace db

} // namespace burger

#endif // MYSQLUTIL_H


