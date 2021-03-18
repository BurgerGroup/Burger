#ifndef MYSQLMANAGER_H
#define MYSQLMANAGER_H

#include <memory>
#include <map>
#include <mutex>
#include <list>
#include "DB.h"
#include "burger/base/Singleton.h"

// todo mysql链接线程池

namespace burger {
namespace db {

class MySQL;

class MySQLManager {
public:
    MySQLManager();
    ~MySQLManager();

};

using MySQLMgrSig = burger::Singleton<MySQLManager>; 


} // namespace db

    
} // namespace burger





#endif // MYSQLMANAGER_H