#ifndef MYSQLMANAGER_H
#define MYSQLMANAGER_H

#include <memory>
#include <map>
#include <mutex>
#include <list>
#include "DB.h"
#include "burger/base/Singleton.h"


namespace burger {
namespace db {

class MySQL;

class MySQLManager {
public:
    MySQLManager();
    ~MySQLManager();

    MySQL::ptr get(const std::string& name);
    void registerMySQL(const std::string& name, const std::map<std::string, std::string>& params);

    void checkConnection(int sec = 30);

    uint32_t getMaxConn() const { return maxConn_;}
    void setMaxConn(uint32_t v) { maxConn_ = v;}

    int execute(const std::string& name, const char* format, ...);
    int execute(const std::string& name, const char* format, va_list ap);
    int execute(const std::string& name, const std::string& sql);

    ISQLData::ptr query(const std::string& name, const char* format, ...);
    ISQLData::ptr query(const std::string& name, const char* format, va_list ap); 
    ISQLData::ptr query(const std::string& name, const std::string& sql);

    MySQLTransaction::ptr openTransaction(const std::string& name, bool auto_commit);
private:
    void freeMySQL(const std::string& name, MySQL* m);
private:
    uint32_t maxConn_;
    std::mutex mutex_;
    std::map<std::string, std::list<MySQL*> > conns_;
    std::map<std::string, std::map<std::string, std::string> > dbDefines;
};

using MySQLMgrSig = burger::Singleton<MySQLManager>; 


} // namespace db

    
} // namespace burger





#endif // MYSQLMANAGER_H