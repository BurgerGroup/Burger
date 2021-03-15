#ifndef MYSQLRES_H
#define MYSQLRES_H

#include "Mysql.h"

namespace burger {
namespace db {
    
class MySQLRes {
public:
    using ptr = std::shared_ptr<MySQLRes>;
    using dataCb = std::function<bool(MYSQL_ROW row
                ,uint64_t columnNum, int row_no)>;
    MySQLRes(MYSQL_RES* res, int eno, const char* estr);

    MYSQL_RES* get() const { return data_.get(); }

    int getErrno() const { return errno_;}
    const std::string& getErrStr() const { return errStr_;}

    bool foreach(dataCb cb);
    bool next();
    uint64_t getRowCount();
    int getColumnCount();
    unsigned long getColumnBytes(int idx);
    // todo *filed
    // int getColumnType(int idx);
    std::string getColumnName(size_t idx);

    bool isNull(int idx);
    // todo: 需要丰富下类型
    int64_t getInt(int idx);
    double getDouble(int idx);
    std::string getString(int idx);
    // std::string getBlob(int idx);
    // time_t getTime(int idx);

private:
    int errno_;
    std::string errStr_;
    MYSQL_ROW cur_;
    unsigned long* curLength_;
    std::shared_ptr<MYSQL_RES> data_;
    std::vector<MYSQL_FIELD *> column_;
};

} // namespace db

} // namespace burget

#endif // MYSQLRES_H