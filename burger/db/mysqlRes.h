#ifndef MYSQLRES_H
#define MYSQLRES_H

#include <memory>
#include <functional>
#include "DB.h"

namespace burger {
namespace db {
    
class MySQLRes : public ISQLData {
public:
    typedef std::shared_ptr<MySQLRes> ptr;
    typedef std::function<bool(MYSQL_ROW row
                ,int field_count, int row_no)> data_cb;
    MySQLRes(MYSQL_RES* res, int eno, const char* estr);

    MYSQL_RES* get() const { return data_.get();}

    int getErrno() const { return errno_;}
    const std::string& getErrStr() const { return errstr_;}

    bool foreach(data_cb cb);

    int getDataCount() override;
    int getColumnCount() override;
    int getColumnBytes(int idx) override;
    int getColumnType(int idx) override;
    std::string getColumnName(int idx) override;

    bool isNull(int idx) override;
    int8_t getInt8(int idx) override;
    uint8_t getUint8(int idx) override;
    int16_t getInt16(int idx) override;
    uint16_t getUint16(int idx) override;
    int32_t getInt32(int idx) override;
    uint32_t getUint32(int idx) override;
    int64_t getInt64(int idx) override;
    uint64_t getUint64(int idx) override;
    float getFloat(int idx) override;
    double getDouble(int idx) override;
    std::string getString(int idx) override;
    std::string getBlob(int idx) override;
    time_t getTime(int idx) override;
    bool next() override;
private:
    int errno_;
    std::string errstr_;
    MYSQL_ROW cur_;
    unsigned long* curLength_;
    std::shared_ptr<MYSQL_RES> data_;
};

} // namespace db

} // namespace burget



#endif // MYSQLRES_H