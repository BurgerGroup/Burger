#include "MysqlRes.h"

using namespace burger;
using namespace burger::db;


MySQLRes::MySQLRes(MYSQL_RES* res, int eno, const char* estr)
    :errno_(eno)
    ,errstr_(estr)
    ,cur_(nullptr)
    ,curLength_(nullptr) {
    if(res) {
        data_.reset(res, mysql_free_result);
    }
}

bool MySQLRes::foreach(data_cb cb) {
    MYSQL_ROW row;
    uint64_t fields = getColumnCount();
    int i = 0;
    while((row = mysql_fetch_row(data_.get()))) {
        if(!cb(row, fields, i++)) {
            break;
        }
    }
    return true;
}

int MySQLRes::getDataCount() {
    return mysql_num_rows(data_.get());
}

int MySQLRes::getColumnCount() {
    return mysql_num_fields(data_.get());
}

int MySQLRes::getColumnBytes(int idx) {
    return curLength_[idx];
}

int MySQLRes::getColumnType(int idx) {
    return 0;
}

std::string MySQLRes::getColumnName(int idx) {
    return "";
}

bool MySQLRes::isNull(int idx) {
    if(cur_[idx] == nullptr) {
        return true;
    }
    return false;
}

int8_t MySQLRes::getInt8(int idx) {
    return getInt64(idx);
}

uint8_t MySQLRes::getUint8(int idx) {
    return getInt64(idx);
}

int16_t MySQLRes::getInt16(int idx) {
    return getInt64(idx);
}

uint16_t MySQLRes::getUint16(int idx) {
    return getInt64(idx);
}

int32_t MySQLRes::getInt32(int idx) {
    return getInt64(idx);
}

uint32_t MySQLRes::getUint32(int idx) {
    return getInt64(idx);
}

int64_t MySQLRes::getInt64(int idx) {
    return sylar::TypeUtil::Atoi(cur_[idx]);
}

uint64_t MySQLRes::getUint64(int idx) {
    return getInt64(idx);
}

float MySQLRes::getFloat(int idx) {
    return getDouble(idx);
}

double MySQLRes::getDouble(int idx) {
    return sylar::TypeUtil::Atof(cur_[idx]);
}

std::string MySQLRes::getString(int idx) {
    return std::string(cur_[idx], cur_Length[idx]);
}

std::string MySQLRes::getBlob(int idx) {
    return std::string(cur_[idx], cur_Length[idx]);
}

time_t MySQLRes::getTime(int idx) {
    if(!cur_[idx]) {
        return 0;
    }
    return sylar::Str2Time(cur_[idx]);
}

bool MySQLRes::next() {
    cur_ = mysql_fetch_row(data_.get());
    cur_Length = mysql_fetch_lengths(data_.get());
    return cur_;
}
