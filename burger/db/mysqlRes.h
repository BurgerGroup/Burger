#ifndef MYSQLRES_H
#define MYSQLRES_H

#include <memory>
#include <functional>
#include "db.h"

namespace burger {
namespace db {
    
class MySQLRes : public ISQLData {
public:
    typedef std::shared_ptr<MySQLRes> ptr;
    typedef std::function<bool(MYSQL_ROW row
                ,int field_count, int row_no)> data_cb;
    MySQLRes(MYSQL_RES* res, int eno, const char* estr);

    MYSQL_RES* get() const { return m_data.get();}

    int getErrno() const { return m_errno;}
    const std::string& getErrStr() const { return m_errstr;}

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
    int m_errno;
    std::string m_errstr;
    MYSQL_ROW m_cur;
    unsigned long* m_curLength;
    std::shared_ptr<MYSQL_RES> m_data;
};

} // namespace db

} // namespace burget



#endif // MYSQLRES_H