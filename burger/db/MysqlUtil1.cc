
#include "MysqlUtil.h"

using namespace burger; 
using namespace burger::db;

ISQLData::ptr MySQLUtil::Query(const std::string& name, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    auto rpy = Query(name, format, ap);
    va_end(ap);
    return rpy;
}

ISQLData::ptr MySQLUtil::Query(const std::string& name, const char* format,va_list ap) {
    auto m = MySQLMgr::GetInstance()->get(name);
    if(!m) {
        return nullptr;
    }
    return m->query(format, ap);
}

ISQLData::ptr MySQLUtil::Query(const std::string& name, const std::string& sql) {
    auto m = MySQLMgr::GetInstance()->get(name);
    if(!m) {
        return nullptr;
    }
    return m->query(sql);
}

ISQLData::ptr MySQLUtil::TryQuery(const std::string& name, uint32_t count, const char* format, ...) {
    for(uint32_t i = 0; i < count; ++i) {
        va_list ap;
        va_start(ap, format);
        auto rpy = Query(name, format, ap);
        va_end(ap);
        if(rpy) {
            return rpy;
        }
    }
    return nullptr;
}

ISQLData::ptr MySQLUtil::TryQuery(const std::string& name, uint32_t count, const std::string& sql) {
    for(uint32_t i = 0; i < count; ++i) {
        auto rpy = Query(name, sql);
        if(rpy) {
            return rpy;
        }
    }
    return nullptr;

}

int MySQLUtil::Execute(const std::string& name, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    auto rpy = Execute(name, format, ap);
    va_end(ap);
    return rpy;
}

int MySQLUtil::Execute(const std::string& name, const char* format, va_list ap) {
    auto m = MySQLMgr::GetInstance()->get(name);
    if(!m) {
        return -1;
    }
    return m->execute(format, ap);
}

int MySQLUtil::Execute(const std::string& name, const std::string& sql) {
    auto m = MySQLMgr::GetInstance()->get(name);
    if(!m) {
        return -1;
    }
    return m->execute(sql);

}

int MySQLUtil::TryExecute(const std::string& name, uint32_t count, const char* format, ...) {
    int rpy = 0;
    for(uint32_t i = 0; i < count; ++i) {
        va_list ap;
        va_start(ap, format);
        rpy = Execute(name, format, ap);
        va_end(ap);
        if(!rpy) {
            return rpy;
        }
    }
    return rpy;
}

int MySQLUtil::TryExecute(const std::string& name, uint32_t count, const std::string& sql) {
    int rpy = 0;
    for(uint32_t i = 0; i < count; ++i) {
        rpy = Execute(name, sql);
        if(!rpy) {
            return rpy;
        }
    }
    return rpy;
}