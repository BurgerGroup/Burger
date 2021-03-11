#ifndef MYSQL_H
#define MYSQL_H


#include <mysql/mysql.h>
#include <memory>
#include <functional>
#include <map>
#include <vector>
#include "db.h"
#include "burger/base/Timestamp.h"
#include "burger/base/Log.h"
#include "burger/base/Util.h"

namespace burger {
namespace db {

class MySQL : public IDB
              ,public std::enable_shared_from_this<MySQL> {
friend class MySQLManager;
public:
    using ptr = std::shared_ptr<MySQL>;

    MySQL(const std::map<std::string, std::string>& args);

    bool connect();
    bool ping();

    // virtual int execute(const char* format, ...) override;
    // int execute(const char* format, va_list ap);
    // virtual int execute(const std::string& sql) override;
    // int64_t getLastInsertId() override;
    // std::shared_ptr<MySQL> getMySQL();
    std::shared_ptr<MYSQL> getRaw() { return mysql_; }

    // uint64_t getAffectedRows();

    // ISQLData::ptr query(const char* format, ...) override;
    // ISQLData::ptr query(const char* format, va_list ap); 
    // ISQLData::ptr query(const std::string& sql) override;
    IStmt::ptr prepare(const std::string& sql) override;
    ITransaction::ptr openTransaction(bool auto_commit) override;
    // template<typename... Args>
    // int execStmt(const char* stmt, Args&&... args);

    // template<class... Args>
    // ISQLData::ptr queryStmt(const char* stmt, Args&&... args);

    // const char* cmd();

    // bool use(const std::string& dbname);
    int getErrno() override;
    std::string getErrStr() override;
    // uint64_t getInsertId();
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