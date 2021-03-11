#ifndef MYSQLMANAGER_H
#define MYSQLMANAGER_H

#include <memory>
#include <map>
#include "db.h"
#include "burger/base/Singleton.h"

class MySQLManager {
public:
    typedef sylar::Mutex MutexType;

    MySQLManager();
    ~MySQLManager();

    MySQL::ptr get(const std::string& name);
    void registerMySQL(const std::string& name, const std::map<std::string, std::string>& params);

    void checkConnection(int sec = 30);

    uint32_t getMaxConn() const { return m_maxConn;}
    void setMaxConn(uint32_t v) { m_maxConn = v;}

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
    uint32_t m_maxConn;
    MutexType m_mutex;
    std::map<std::string, std::list<MySQL*> > m_conns;
    std::map<std::string, std::map<std::string, std::string> > m_dbDefines;
};

using burger::Singleton<MySQLManager> MySQLMgrSig; 


#endif // MYSQLMANAGER_H