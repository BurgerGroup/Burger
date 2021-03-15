#ifndef MYSQL_H
#define MYSQL_H

#include <memory>
#include <functional>
#include <map>
#include <vector>
#include <string>

#include <mysql/mysql.h>
#include "burger/base/Timestamp.h"
#include "burger/base/Log.h"
#include "burger/base/Util.h"

namespace burger {
namespace db {

class MySQLRes;
class MySQLStmt;
// class MySQLManager;

class MySQL : public std::enable_shared_from_this<MySQL> {
// friend class MySQLManager;
public:
    using ptr = std::shared_ptr<MySQL>;
    MySQL(const std::map<std::string, std::string>& args);

    bool connect();
    bool ping();
    
    void mysqlInfo();

    int execute(const std::string& sql);
    int64_t getLastInsertId();
    std::shared_ptr<MySQL> getMySQL();
    std::shared_ptr<MYSQL> getRaw() { return mysql_; }

    uint64_t getAffectedRows();

    // todo : 变长参数
    std::shared_ptr<MySQLRes> query(const std::string& sql);
    std::shared_ptr<MySQLStmt> prepare(const std::string& sql);
    // MySQLTransaction::ptr openTransaction(bool auto_commit) override;

    const char* cmd() { return cmd_.c_str(); }

    bool use(const std::string& dbname);
    int getErrno();
    std::string getErrStr();
    uint64_t getInsertId();
private:
    bool isNeedCheck();
private:
    std::map<std::string, std::string> params_;
    std::shared_ptr<MYSQL> mysql_;
    
    std::string cmd_;
    std::string dbname_;

    Timestamp lastUsedTime_;
    bool hasError_;
    int poolSize_;
};


} // namespace db

} // namespace burger




#endif // MYSQL_H