#ifndef MYSQLTRANSACTION_H
#define MYSQLTRANSACTION_H

#include <memory>
#include "db.h"

namespace burger {
namespace db {

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

    bool isAutoCommit() const { return m_autoCommit;}
    bool isFinished() const { return m_isFinished;}
    bool isError() const { return m_hasError;}
private:
    MySQLTransaction(MySQL::ptr mysql, bool auto_commit);
private:
    MySQL::ptr m_mysql;
    bool m_autoCommit;
    bool m_isFinished;
    bool m_hasError;
};

} // namespace db

} // namespace burger



#endif // MYSQLTRANSACTION_H