#ifndef MYSQLTRANSACTION_H
#define MYSQLTRANSACTION_H

#include <memory>
#include "DB.h"

namespace burger {
namespace db {

class MySQL;

class MySQLTransaction : public ITransaction {
public:
    typedef std::shared_ptr<MySQLTransaction> ptr;

    static MySQLTransaction::ptr Create(MySQL::ptr mysql, bool auto_commit);
    ~MySQLTransaction();

    bool begin() override;
    bool commit() override;
    bool rollback() override;

    virtual int execute(const char* format, ...) override;
    int execute(const char* format, va_list ap);
    virtual int execute(const std::string& sql) override;
    int64_t getLastInsertId() override;
    std::shared_ptr<MySQL> getMySQL();

    bool isAutoCommit() const { return autoCommit_;}
    bool isFinished() const { return isFinished_;}
    bool isError() const { return hasError_;}
private:
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