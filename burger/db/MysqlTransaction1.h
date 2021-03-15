#ifndef MYSQLTRANSACTION_H
#define MYSQLTRANSACTION_H

#include <memory>
#include "Mysql.h"

namespace burger {
namespace db {

class MySQL;

class MySQLTransaction {
public:
    using ptr = std::shared_ptr<MySQLTransaction>;

    static MySQLTransaction::ptr Create(MySQL::ptr mysql, bool auto_commit);
    ~MySQLTransaction();

    bool begin() override;
    bool commit() override;
    bool rollback() override;

    // todo: add 变长参数
    int execute(const std::string& sql);
    int64_t getLastInsertId();
    std::shared_ptr<MySQL> getMySQL();

    bool isAutoCommit() const { return autoCommit_;}
    bool isFinished() const { return isFinished_;}
    bool isError() const { return hasError_;}
// private:
    MySQLTransaction(MySQL::ptr mysql, bool auto_commit);
private:
    MySQL::ptr mysql_;
    bool autoCommit_;
    bool isFinished_;
    bool hasError_;
};

} // namespace db

} // namespace burger



#endif // MYSQLTRANSACTION_H