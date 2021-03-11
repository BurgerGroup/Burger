#ifndef MYSQLSTMT_H
#define MYSQLSTMT_H

#include "db.h"
#include "mysql.h"

namespace burger {
namespace db {

class MySQLStmt : public IStmt
                  ,public std::enable_shared_from_this<MySQLStmt> {
public:
    using ptr = std::shared_ptr<MySQLStmt>;
    static MySQLStmt::ptr Create(MySQL::ptr db, const std::string& stmt);

    ~MySQLStmt();
    int bind(int idx, const int8_t& value);
    int bind(int idx, const uint8_t& value);
    int bind(int idx, const int16_t& value);
    int bind(int idx, const uint16_t& value);
    int bind(int idx, const int32_t& value);
    int bind(int idx, const uint32_t& value);
    int bind(int idx, const int64_t& value);
    int bind(int idx, const uint64_t& value);
    int bind(int idx, const float& value);
    int bind(int idx, const double& value);
    int bind(int idx, const std::string& value);
    int bind(int idx, const char* value);
    int bind(int idx, const void* value, int len);
    //int bind(int idx, const MYSQL_TIME& value, int type = MYSQL_TYPE_TIMESTAMP);
    //for null type
    int bind(int idx);

    int bindInt8(int idx, const int8_t& value) override;
    int bindUint8(int idx, const uint8_t& value) override;
    int bindInt16(int idx, const int16_t& value) override;
    int bindUint16(int idx, const uint16_t& value) override;
    int bindInt32(int idx, const int32_t& value) override;
    int bindUint32(int idx, const uint32_t& value) override;
    int bindInt64(int idx, const int64_t& value) override;
    int bindUint64(int idx, const uint64_t& value) override;
    int bindFloat(int idx, const float& value) override;
    int bindDouble(int idx, const double& value) override;
    int bindString(int idx, const char* value) override;
    int bindString(int idx, const std::string& value) override;
    int bindBlob(int idx, const void* value, int64_t size) override;
    int bindBlob(int idx, const std::string& value) override;
    //int bindTime(int idx, const MYSQL_TIME& value, int type = MYSQL_TYPE_TIMESTAMP);
    int bindTime(int idx, const time_t& value) override;
    int bindNull(int idx) override;

    int getErrno() override;
    std::string getErrStr() override;

    int execute() override;
    int64_t getLastInsertId() override;
    ISQLData::ptr query() override;

    // todo why not smart ptr?
    MYSQL_STMT* getRaw() const { return stmt_; }
private:
    // todo 为何不能在栈上生成
    MySQLStmt(MySQL::ptr db, MYSQL_STMT* stmt);
private:
    MySQL::ptr mysql_;
    MYSQL_STMT* stmt_;
    // todo 仔细阅读 MYSQL_BIND 文档
    // 用来绑定语句的参数。可以做输出，也可以做输入。
    // 如：insert into test (flag,type,f_index) values (1,?,?) 这样的语句，后两个参数会根据数据不同而发生变化。
    std::vector<MYSQL_BIND> binds_;
};




} // namespace db

} // namespace burger



#endif // MYSQLSTMT_H