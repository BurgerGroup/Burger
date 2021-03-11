#include "mysqlStmt.h"

using namespace burger;
using namespace burger::db;

MySQLStmt::ptr MySQLStmt::Create(MySQL::ptr db, const std::string& stmt) {
    // MYSQL_STMT *mysql_stmt_init(MYSQL *mysql)  创建并返回MYSQL_STMT处理程序
    auto st = mysql_stmt_init(db->getRaw().get());
    if(!st) {
        ERROR("return nullptr because of lacking memory");
        return nullptr;
    }
    // int mysql_stmt_prepare(MYSQL_STMT *stmt, const char *stmt_str, unsigned long length)
    //调用mysql_stmt_prepare()对SQL语句进行预处理  https://www.docs4dev.com/docs/zh/mysql/5.7/reference/mysql-stmt-prepare.html
    if(mysql_stmt_prepare(st, stmt.c_str(), stmt.size())) {
        ERROR("stmt = {} errno = {} errstr = {}",
            stmt, mysql_stmt_errno(st), mysql_stmt_error(st));
        mysql_stmt_close(st);
        return nullptr;
    }
    // unsigned long mysql_stmt_param_count(MYSQL_STMT *stmt)
    // 返回准备好的语句中存在的参数标记的数量。
    int count = mysql_stmt_param_count(st);
    MySQLStmt::ptr rt = std::make_shared<MySQLStmt>(db, st);
    rt->binds_.resize(count);
    // todo
    memset(&rt->binds_[0], 0, sizeof(rt->binds_[0]) * count);
    return rt;
}

MySQLStmt::~MySQLStmt() {
    if(stmt_) {
        mysql_stmt_close(stmt_);
    }

    // TODO 为何这里需要free， 里面的buf?
    for(auto& i : binds_) {
        if(i.buffer) {
            free(i.buffer);
        }
    }
}

MySQLStmt::MySQLStmt(MySQL::ptr db, MYSQL_STMT* stmt)
    : mysql_(db),
    stmt_(stmt)  {
}


int MySQLStmt::bind(int idx, const int8_t& value) {
    return bindInt8(idx, value);
}

int MySQLStmt::bind(int idx, const uint8_t& value) {
    return bindUint8(idx, value);
}

int MySQLStmt::bind(int idx, const int16_t& value) {
    return bindInt16(idx, value);
}

int MySQLStmt::bind(int idx, const uint16_t& value) {
    return bindUint16(idx, value);
}

int MySQLStmt::bind(int idx, const int32_t& value) {
    return bindInt32(idx, value);
}

int MySQLStmt::bind(int idx, const uint32_t& value) {
    return bindUint32(idx, value);
}

int MySQLStmt::bind(int idx, const int64_t& value) {
    return bindInt64(idx, value);
}

int MySQLStmt::bind(int idx, const uint64_t& value) {
    return bindUint64(idx, value);
}

int MySQLStmt::bind(int idx, const float& value) {
    return bindFloat(idx, value);
}

int MySQLStmt::bind(int idx, const double& value) {
    return bindDouble(idx, value);
}

int MySQLStmt::bind(int idx, const std::string& value) {
    return bindString(idx, value);
}

int MySQLStmt::bind(int idx, const char* value) {
    return bindString(idx, value);
}

int MySQLStmt::bind(int idx, const void* value, int len) {
    return bindBlob(idx, value, len);
}

int MySQLStmt::bind(int idx) {
    idx -= 1;
    binds_[idx].buffer_type = MYSQL_TYPE_NULL;
    return 0;
}


int MySQLStmt::bindInt8(int idx, const int8_t& value) {
    idx -= 1;
    binds_[idx].buffer_type = MYSQL_TYPE_TINY;
#define BIND_COPY(ptr, size) \
    if(binds_[idx].buffer == nullptr) { \
        binds_[idx].buffer = malloc(size); \
    } \
    memcpy(binds_[idx].buffer, ptr, size);
    BIND_COPY(&value, sizeof(value));
    binds_[idx].is_unsigned = false;
    binds_[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindUint8(int idx, const uint8_t& value) {
    idx -= 1;
    binds_[idx].buffer_type = MYSQL_TYPE_TINY;
    BIND_COPY(&value, sizeof(value));
    binds_[idx].is_unsigned = true;
    binds_[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindInt16(int idx, const int16_t& value) {
    idx -= 1;
    binds_[idx].buffer_type = MYSQL_TYPE_SHORT;
    BIND_COPY(&value, sizeof(value));
    binds_[idx].is_unsigned = false;
    binds_[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindUint16(int idx, const uint16_t& value) {
    idx -= 1;
    binds_[idx].buffer_type = MYSQL_TYPE_SHORT;
    BIND_COPY(&value, sizeof(value));
    binds_[idx].is_unsigned = true;
    binds_[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindInt32(int idx, const int32_t& value) {
    idx -= 1;
    binds_[idx].buffer_type = MYSQL_TYPE_LONG;
    BIND_COPY(&value, sizeof(value));
    binds_[idx].is_unsigned = false;
    binds_[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindUint32(int idx, const uint32_t& value) {
    idx -= 1;
    binds_[idx].buffer_type = MYSQL_TYPE_LONG;
    BIND_COPY(&value, sizeof(value));
    binds_[idx].is_unsigned = true;
    binds_[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindInt64(int idx, const int64_t& value) {
    idx -= 1;
    binds_[idx].buffer_type = MYSQL_TYPE_LONGLONG;
    BIND_COPY(&value, sizeof(value));
    binds_[idx].is_unsigned = false;
    binds_[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindUint64(int idx, const uint64_t& value) {
    idx -= 1;
    binds_[idx].buffer_type = MYSQL_TYPE_LONGLONG;
    BIND_COPY(&value, sizeof(value));
    binds_[idx].is_unsigned = true;
    binds_[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindFloat(int idx, const float& value) {
    idx -= 1;
    binds_[idx].buffer_type = MYSQL_TYPE_FLOAT;
    BIND_COPY(&value, sizeof(value));
    binds_[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindDouble(int idx, const double& value) {
    idx -= 1;
    binds_[idx].buffer_type = MYSQL_TYPE_DOUBLE;
    BIND_COPY(&value, sizeof(value));
    binds_[idx].buffer_length = sizeof(value);
    return 0;
}

int MySQLStmt::bindString(int idx, const char* value) {
    idx -= 1;
    binds_[idx].buffer_type = MYSQL_TYPE_STRING;
#define BIND_COPY_LEN(ptr, size) \
    if(binds_[idx].buffer == nullptr) { \
        binds_[idx].buffer = malloc(size); \
    } else if((size_t)binds_[idx].buffer_length < (size_t)size) { \
        free(binds_[idx].buffer); \
        binds_[idx].buffer = malloc(size); \
    } \
    memcpy(binds_[idx].buffer, ptr, size); \
    binds_[idx].buffer_length = size;
    BIND_COPY_LEN(value, strlen(value));
    return 0;
}

int MySQLStmt::bindString(int idx, const std::string& value) {
    idx -= 1;
    binds_[idx].buffer_type = MYSQL_TYPE_STRING;
    BIND_COPY_LEN(value.c_str(), value.size());
    return 0;
}

int MySQLStmt::bindBlob(int idx, const void* value, int64_t size) {
    idx -= 1;
    binds_[idx].buffer_type = MYSQL_TYPE_BLOB;
    BIND_COPY_LEN(value, size);
    return 0;
}

int MySQLStmt::bindBlob(int idx, const std::string& value) {
    idx -= 1;
    binds_[idx].buffer_type = MYSQL_TYPE_BLOB;
    BIND_COPY_LEN(value.c_str(), value.size());
    return 0;
}

int MySQLStmt::bindNull(int idx) {
    return bind(idx);
}


int MySQLStmt::getErrno() {
    return mysql_stmt_errno(stmt_);
}

std::string MySQLStmt::getErrStr() {
    const char* e = mysql_stmt_error(stmt_);
    if(e) {
        return e;
    }
    return "";
}

// need fix
// int MySQLStmt::bindTime(int idx, const time_t& value) {
//     return bindString(idx, sylar::Time2Str(value));
// }
