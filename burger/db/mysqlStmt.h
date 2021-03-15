#ifndef MYSQLSTMT_H
#define MYSQLSTMT_H

#include "Mysql.h"

namespace burger {
namespace db {

class MySQLStmtRes;

class MySQLStmt : public std::enable_shared_from_this<MySQLStmt> {
public:
    using ptr = std::shared_ptr<MySQLStmt>;
    static MySQLStmt::ptr Create(std::shared_ptr<MySQL> db, const std::string& stmt);

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
    int bind(int idx); //for null type

    int bindInt8(int idx, const int8_t& value);
    int bindUint8(int idx, const uint8_t& value);
    int bindInt16(int idx, const int16_t& value);
    int bindUint16(int idx, const uint16_t& value);
    int bindInt32(int idx, const int32_t& value);
    int bindUint32(int idx, const uint32_t& value);
    int bindInt64(int idx, const int64_t& value);
    int bindUint64(int idx, const uint64_t& value);
    int bindFloat(int idx, const float& value);
    int bindDouble(int idx, const double& value);
    int bindString(int idx, const char* value);
    int bindString(int idx, const std::string& value);
    int bindBlob(int idx, const void* value, int64_t size);
    int bindBlob(int idx, const std::string& value);
    // int bindTime(int idx, const MYSQL_TIME& value, int type = MYSQL_TYPE_TIMESTAMP);
    // int bindTime(int idx, const time_t& value);
    int bindNull(int idx);

    int getErrno();
    std::string getErrStr();

    int execute();
    int64_t getLastInsertId();
    std::shared_ptr<MySQLStmtRes> query();

    // todo why not smart ptr?
    MYSQL_STMT* getRaw() const { return stmt_; }
// private:
    // todo : 这里加了private报错, 如何解决
    // todo 为何不能在栈上生成 -- shared_ptr? 工厂模式生成
    MySQLStmt(std::shared_ptr<MySQL> db, MYSQL_STMT* stmt);
private:
    std::shared_ptr<MySQL> mysql_;
    MYSQL_STMT* stmt_;  // todo : why not smart ptr?
    // 用来绑定语句的参数。可以做输出，也可以做输入。
    // 如：insert into test (flag,type,f_index) values (1,?,?) 这样的语句，后两个参数会根据数据不同而发生变化。
    std::vector<MYSQL_BIND> binds_;
};




} // namespace db

} // namespace burger



#endif // MYSQLSTMT_H