#include "MysqlStmt.h"
#include "MysqlStmtRes.h"

using namespace burger;
using namespace burger::db;

MySQLStmt::ptr MySQLStmt::Create(std::shared_ptr<MySQL> db, const std::string& stmt) {
    // MYSQL_STMT *mysql_stmt_init(MYSQL *mysql)  创建并返回MYSQL_STMT处理程序
    auto st = mysql_stmt_init(db->getRaw().get());
    if(!st) {
        ERROR("return nullptr because of lacking memory");
        return nullptr;
    }
    // int mysql_stmt_prepare(MYSQL_STMT *stmt, const char *stmt_str, unsigned long length)
    // 调用mysql_stmt_prepare()对SQL语句进行预处理  https://www.docs4dev.com/docs/zh/mysql/5.7/reference/mysql-stmt-prepare.html
    if(mysql_stmt_prepare(st, stmt.c_str(), stmt.size())) {
        ERROR("stmt = {} errno = {} errstr = {}",
            stmt, mysql_stmt_errno(st), mysql_stmt_error(st));
        mysql_stmt_close(st);
        return nullptr;
    }
    // unsigned long mysql_stmt_param_count(MYSQL_STMT *stmt)
    // 返回准备好的语句中存在的参数标记的数量。
    int count = static_cast<int>(mysql_stmt_param_count(st));
    MySQLStmt::ptr rt = std::make_shared<MySQLStmt>(db, st);
    rt->binds_.resize(count);
    memset(&rt->binds_[0], 0, sizeof(rt->binds_[0]) * count);
    return rt;
}

MySQLStmt::~MySQLStmt() {
    if(stmt_) {
        mysql_stmt_close(stmt_);
    }

    // todo : 能更自动管理这里的buffer吗
    for(auto& i : binds_) {
        if(i.buffer) {
            free(i.buffer);
        }
    }
}

MySQLStmt::MySQLStmt(std::shared_ptr<MySQL> db, MYSQL_STMT* stmt)
    : mysql_(db),
    stmt_(stmt)  {
}

// https://www.mysqlzh.com/doc/196/113.html 各种类型对应
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
    // todo can we avoid use malloc ? more modern way
    // https://stackoverflow.com/questions/14111900/using-new-on-void-pointer
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
    } else if(binds_[idx].buffer_length < static_cast<unsigned long>(size)) { \
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

int MySQLStmt::execute() {
    // my_bool mysql_stmt_bind_param(MYSQL_STMT *stmt, MYSQL_BIND *bind)
    // 用于为SQL语句中的参数标记符绑定数据。
    mysql_stmt_bind_param(stmt_, &binds_[0]);
    // int mysql_stmt_execute(MYSQL_STMT *stmt)
    return mysql_stmt_execute(stmt_);    
}

int64_t MySQLStmt::getLastInsertId() {
    // my_ulonglong mysql_stmt_insert_id(MYSQL_STMT *stmt)
    // 对于预处理语句的AUTO_INCREMENT列，返回生成的ID。
    return mysql_stmt_insert_id(stmt_);
}

std::shared_ptr<MySQLStmtRes> MySQLStmt::query()  {
    mysql_stmt_bind_param(stmt_, &binds_[0]);
    return MySQLStmtRes::Create(shared_from_this());
}








// need fix
// int MySQLStmt::bindTime(int idx, const time_t& value) {
//     return bindString(idx, sylar::Time2Str(value));
// }
