#include "mysqlStmtRes.h"
#include "mysqlStmt.h"

using namespace burger;
using namespace burger::db;

MySQLStmtRes::ptr MySQLStmtRes::Create(std::shared_ptr<MySQLStmt> stmt) {
    int eno = mysql_stmt_errno(stmt->getRaw());
    const char* errstr = mysql_stmt_error(stmt->getRaw());
    MySQLStmtRes::ptr rt(new MySQLStmtRes(stmt, eno, errstr));
    if(eno) {
        return rt;
    }
    MYSQL_RES* res = mysql_stmt_result_metadata(stmt->getRaw());
    if(!res) {
        return MySQLStmtRes::ptr(new MySQLStmtRes(stmt, stmt->getErrno()
                                 ,stmt->getErrStr()));
    }

    int num = mysql_num_fields(res);
    MYSQL_FIELD* fields = mysql_fetch_fields(res);

    rt->binds_.resize(num);
    memset(&rt->binds_[0], 0, sizeof(rt->binds_[0]) * num);
    rt->datas_.resize(num);

    for(int i = 0; i < num; ++i) {
        rt->datas_[i].type = fields[i].type;
        switch(fields[i].type) {
#define XX(m, t) \
            case m: \
                rt->datas_[i].alloc(sizeof(t)); \
                break;
            XX(MYSQL_TYPE_TINY, int8_t);
            XX(MYSQL_TYPE_SHORT, int16_t);
            XX(MYSQL_TYPE_LONG, int32_t);
            XX(MYSQL_TYPE_LONGLONG, int64_t);
            XX(MYSQL_TYPE_FLOAT, float);
            XX(MYSQL_TYPE_DOUBLE, double);
            XX(MYSQL_TYPE_TIMESTAMP, MYSQL_TIME);
            XX(MYSQL_TYPE_DATETIME, MYSQL_TIME);
            XX(MYSQL_TYPE_DATE, MYSQL_TIME);
            XX(MYSQL_TYPE_TIME, MYSQL_TIME);
#undef XX
            default:
                rt->datas_[i].alloc(fields[i].length);
                break;
        }

        rt->binds_[i].buffer_type = rt->datas_[i].type;
        rt->binds_[i].buffer = rt->datas_[i].data;
        rt->binds_[i].buffer_length = rt->datas_[i].data_length;
        rt->binds_[i].length = &rt->datas_[i].length;
        rt->binds_[i].is_null = &rt->datas_[i].is_null;
        rt->binds_[i].error = &rt->datas_[i].error;
    }

    if(mysql_stmt_bind_result(stmt->getRaw(), &rt->binds_[0])) {
        return MySQLStmtRes::ptr(new MySQLStmtRes(stmt, stmt->getErrno()
                                    , stmt->getErrStr()));
    }

    stmt->execute();

    if(mysql_stmt_store_result(stmt->getRaw())) {
        return MySQLStmtRes::ptr(new MySQLStmtRes(stmt, stmt->getErrno()
                                    , stmt->getErrStr()));
    }
    //rt->next();
    return rt;
}

int MySQLStmtRes::getDataCount() {
    return mysql_stmt_num_rows(stmt_->getRaw());
}

int MySQLStmtRes::getColumnCount() {
    return mysql_stmt_field_count(stmt_->getRaw());
}

int MySQLStmtRes::getColumnBytes(int idx) {
    return datas_[idx].length;
}

int MySQLStmtRes::getColumnType(int idx) {
    return datas_[idx].type;
}

std::string MySQLStmtRes::getColumnName(int idx) {
    return "";
}

bool MySQLStmtRes::isNull(int idx) {
    return datas_[idx].is_null;
}

#define XX(type) \
    return *static_cast<type*>datas_[idx].data
    // return *(type*)datas_[idx].data  // old style cast
int8_t MySQLStmtRes::getInt8(int idx) {
    XX(int8_t);
}

uint8_t MySQLStmtRes::getUint8(int idx) {
    XX(uint8_t);
}

int16_t MySQLStmtRes::getInt16(int idx) {
    XX(int16_t);
}

uint16_t MySQLStmtRes::getUint16(int idx) {
    XX(uint16_t);
}

int32_t MySQLStmtRes::getInt32(int idx) {
    XX(int32_t);
}

uint32_t MySQLStmtRes::getUint32(int idx) {
    XX(uint32_t);
}

int64_t MySQLStmtRes::getInt64(int idx) {
    XX(int64_t);
}

uint64_t MySQLStmtRes::getUint64(int idx) {
    XX(uint64_t);
}

float MySQLStmtRes::getFloat(int idx) {
    XX(float);
}

double MySQLStmtRes::getDouble(int idx) {
    XX(double);
}
#undef XX

std::string MySQLStmtRes::getString(int idx) {
    return std::string(datas_[idx].data, datas_[idx].length);
}

std::string MySQLStmtRes::getBlob(int idx) {
    return std::string(datas_[idx].data, datas_[idx].length);
}

time_t MySQLStmtRes::getTime(int idx) {
    MYSQL_TIME* v = (MYSQL_TIME*)datas_[idx].data;
    time_t ts = 0;
    mysql_time_to_time_t(*v, ts);
    return ts;
}

bool MySQLStmtRes::next() {
    return !mysql_stmt_fetch(stmt_->getRaw());
}

MySQLStmtRes::Data::Data()
    :is_null(0)
    ,error(0)
    ,type()
    ,length(0)
    ,data_length(0)
    ,data(nullptr) {
}

MySQLStmtRes::Data::~Data() {
    if(data) {
        delete[] data;
    }
}

void MySQLStmtRes::Data::alloc(size_t size) {
    if(data) {
        delete[] data;
    }
    data = new char[size]();
    length = size;
    data_length = size;
}

MySQLStmtRes::MySQLStmtRes(std::shared_ptr<MySQLStmt> stmt, int eno
                           ,const std::string& estr)
    :errno_(eno)
    ,errstr_(estr)
    ,stmt_(stmt) {
}

MySQLStmtRes::~MySQLStmtRes() {
    if(!errno_) {
        mysql_stmt_free_result(stmt_->getRaw());
    }
}

