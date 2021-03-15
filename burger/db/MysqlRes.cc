#include "MysqlRes.h"
#include "burger/base/Type.h"
using namespace burger;
using namespace burger::db;


MySQLRes::MySQLRes(MYSQL_RES* res, int eno, const char* estr)
    : errno_(eno),
    errStr_(estr),
    cur_(nullptr),
    curLength_(nullptr) {
    if(res) {
        // void mysql_free_result(MYSQL_RES *result)
        // 释放由mysql_store_result()、mysql_use_result()、mysql_list_dbs()等为结果集分配的内存
        data_.reset(res, mysql_free_result);
    }
}


bool MySQLRes::foreach(dataCb cb) {
    // 这是1行数据的“类型安全”表示, 行是通过调用mysql_fetch_row()获得的。
    MYSQL_ROW row;
    int columnNum = getColumnCount();
    int i = 0;
    // MYSQL_ROW mysql_fetch_row(MYSQL_RES *result)
    // 下一行的MYSQL_ROW结构。如果没有更多要检索的行或出现了错误，返回NULL。
    while((row = mysql_fetch_row(data_.get()))) {
        if(!cb(row, columnNum, i++)) {
            break;
        }
    }
    return true;
}

uint64_t MySQLRes::getRowCount() {
    // my_ulonglong mysql_num_rows(MYSQL_RES *result)
    // 返回结果集中的行数。
    return mysql_num_rows(data_.get());
}

int MySQLRes::getColumnCount() {
    // unsigned int mysql_num_fields(MYSQL_RES *result)
    // 返回结果集中的列数。
    return mysql_num_fields(data_.get());
}

unsigned long MySQLRes::getColumnBytes(int idx) {
    return curLength_[idx];
}

std::string MySQLRes::getColumnName(size_t idx) {
    if(!column_.empty()) return column_[idx]->name;
    MYSQL_FIELD *field;  
    // MYSQL_FIELD *mysql_fetch_field(MYSQL_RES *result)
    while((field = mysql_fetch_field(data_.get()))) {
        column_.push_back(field);
    }
    return idx < column_.size() ? column_[idx]->name : "";
}


bool MySQLRes::isNull(int idx) {
    if(cur_[idx] == nullptr) {
        return true;
    }
    return false;
}

int64_t MySQLRes::getInt(int idx) {
    return TypeUtil::strToInt64(cur_[idx]);
}

double MySQLRes::getDouble(int idx) {
    return TypeUtil::strToDouble(cur_[idx]);
}

std::string MySQLRes::getString(int idx) {
    return std::string(cur_[idx], curLength_[idx]);
}

// std::string MySQLRes::getBlob(int idx) {
//     return std::string(cur_[idx], curLength_[idx]);
// }

// time_t MySQLRes::getTime(int idx) {
//     if(!cur_[idx]) {
//         return 0;
//     }
//     return burger::Str2Time(cur_[idx]);
// }

bool MySQLRes::next() {
    cur_ = mysql_fetch_row(data_.get());
    // unsigned long *mysql_fetch_lengths(MYSQL_RES *result)
    // 返回结果集内当前行的列的长度。
    curLength_ = mysql_fetch_lengths(data_.get());
    return cur_ == nullptr ? false : true;
}
