#include "MysqlTransaction.h"

using namespace burger;
using namespace burger::db;

MySQLTransaction::ptr MySQLTransaction::Create(MySQL::ptr mysql, bool auto_commit) {
    MySQLTransaction::ptr rt = make_shared<MySQLTransaction>(mysql, auto_commit);
    if(rt->begin()) {
        return rt;
    }
    return nullptr;
}

MySQLTransaction::~MySQLTransaction() {
    if(autoCommit_) {
        commit();
    } else {
        rollback();
    }
}

int64_t MySQLTransaction::getLastInsertId() {
    return mysql_->getLastInsertId();
}

bool MySQLTransaction::begin() {
    int rt = execute("BEGIN");
    return rt == 0;
}

bool MySQLTransaction::commit() {
    if(isFinished_ || hasError_) {
        return !hasError_;
    }
    int rt = execute("COMMIT");
    if(rt == 0) {
        isFinished_ = true;
    } else {
        hasError_ = true;
    }
    return rt == 0;
}

bool MySQLTransaction::rollback() {
    if(isFinished_) {
        return true;
    }
    int rt = execute("ROLLBACK");
    if(rt == 0) {
        isFinished_ = true;
    } else {
        hasError_ = true;
    }
    return rt == 0;
}


int MySQLTransaction::execute(const std::string& sql) {
    if(isFinished_) {
        ERROR("transaction is finished, sql={}", sql);
        return -1;
    }
    int rt = mysql_->execute(sql);
    if(rt) {
        hasError_ = true;
    }
    return rt;

}

std::shared_ptr<MySQL> MySQLTransaction::getMySQL() {
    return mysql_;
}

MySQLTransaction::MySQLTransaction(MySQL::ptr mysql, bool auto_commit)
    :mysql_(mysql)
    ,autoCommit_(auto_commit)
    ,isFinished_(false)
    ,hasError_(false) {
}