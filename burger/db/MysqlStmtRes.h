#ifndef MYSQLSTMTRES_H
#define MYSQLSTMTRES_H

#include "Mysql.h"

namespace burger {
namespace db {
class MySQLStmt;

class MySQLStmtRes {
friend class MySQLStmt;
public:
    using ptr = std::shared_ptr<MySQLStmtRes>;
    static MySQLStmtRes::ptr Create(std::shared_ptr<MySQLStmt> stmt);
    ~MySQLStmtRes();

    int getErrno() const { return errno_;}
    const std::string& getErrStr() const { return errstr_;}

    uint64_t getDataCount();
    int getColumnCount();
    int getColumnBytes(int idx);
    int getColumnType(int idx);
    // std::string getColumnName(int idx);

    bool isNull(int idx);
    int8_t getInt8(int idx);
    uint8_t getUint8(int idx);
    int16_t getInt16(int idx);
    uint16_t getUint16(int idx);
    int32_t getInt32(int idx);
    uint32_t getUint32(int idx);
    int64_t getInt64(int idx);
    uint64_t getUint64(int idx);
    float getFloat(int idx);
    double getDouble(int idx);
    std::string getString(int idx);
    std::string getBlob(int idx);
    time_t getTime(int idx);
    bool next();
    MySQLStmtRes(std::shared_ptr<MySQLStmt> stmt, int eno, const std::string& estr);
private:
    // MySQLStmtRes(std::shared_ptr<MySQLStmt> stmt, int eno, const std::string& estr);
    // 与MYSQL_BIND相对应
    struct Data {
        Data();
        ~Data();

        void alloc(size_t size);

        my_bool is_null;
        my_bool error;
        enum_field_types type;
        unsigned long length;
        unsigned long data_length;
        void* data;  // todo : std::vector<char>
    };
private:
    int errno_;
    std::string errstr_;
    std::shared_ptr<MySQLStmt> stmt_;
    std::vector<MYSQL_BIND> binds_;
    std::vector<Data> datas_;
};

} // namespace db

} // namespace burger



#endif // MYSQLSTMTRES_H