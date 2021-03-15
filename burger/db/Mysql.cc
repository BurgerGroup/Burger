#include "Mysql.h"
#include "MysqlRes.h"
#include "MysqlStmt.h"

using namespace burger;
using namespace burger::db;

namespace  {

/*
my_bool mysql_thread_init(void)

todo : 初始化一个线程句柄，应当在线程创建之后立即调用。
*/
struct MySQLThreadIniter {
    MySQLThreadIniter() {
        mysql_thread_init();
    }

    ~MySQLThreadIniter() {
        mysql_thread_end();
    }
};

static MYSQL* mysql_init(std::map<std::string, std::string>& params,
                         const int& timeout) {
    // todo why static
    static thread_local MySQLThreadIniter s_thread_initer;

    // mysql_init 在需要线程安全的程序中初始化全局变量和线程句柄。 
    MYSQL* mysql = ::mysql_init(nullptr);
    if(mysql == nullptr) {
        ERROR("mysql_init error");
        return nullptr;
    }
    // mysql_options 可用于设置额外的连接选项，并影响连接的行为。可多次调用该函数来设置数个选项。
    // 应在mysql_init()之后、以及mysql_connect()或mysql_real_connect()之前调用mysql_options()。
    if(timeout > 0) {
        mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    }
    bool close = false;
    mysql_options(mysql, MYSQL_OPT_RECONNECT, &close);
    mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8mb4");

    int port = util::GetParamValue(params, "port", 0);
    std::string host = util::GetParamValue<std::string>(params, "host");
    std::string user = util::GetParamValue<std::string>(params, "user");
    std::string passwd = util::GetParamValue<std::string>(params, "passwd");
    std::string dbname = util::GetParamValue<std::string>(params, "dbname");
    // mysql_real_connect() 尝试构建与在host上运行的 MySQL 服务器的连接
    if(mysql_real_connect(mysql, host.c_str(), user.c_str(), passwd.c_str()
                          , dbname.c_str(), port, NULL, 0) == nullptr) {
        // mysql_errno()函数获取出错代号
        ERROR("mysql_real_connect({}, {}, {}) error : {}", host, port, dbname, mysql_error(mysql));
        mysql_close(mysql);
        return nullptr;
    }
    return mysql;
}


static MYSQL_RES* my_mysql_query(MYSQL* mysql, const char* sql) {
    if(mysql == nullptr) { 
        ERROR("mysql_query mysql is null")
        return nullptr;
    }

    if(sql == nullptr) {
        ERROR("mysql_query sql is null");
        return nullptr;
    }

    if(::mysql_query(mysql, sql)) {
        ERROR("mysql_query({}) error: {}", sql, mysql_error(mysql));
        return nullptr;
    }
    // MYSQL_RES *mysql_store_result(MYSQL *mysql)
    // 对于成功检索了数据的每个查询（SELECT、SHOW、DESCRIBE、EXPLAIN、CHECK TABLE等），必须调用mysql_store_result()或mysql_use_result() 。
    // mysql_store_result()将查询的全部结果读取到客户端，分配1个MYSQL_RES结构，并将结果置于该结构中。
    MYSQL_RES* res = mysql_store_result(mysql);
    if(res == nullptr) {
        ERROR("mysql_store_result() error: {}", mysql_error(mysql));
    }
    return res;
}
} // namespace 

MySQL::MySQL(const std::map<std::string, std::string>& args)
    : params_(args),
    lastUsedTime_(Timestamp::now()),
    hasError_(false),
    poolSize_(10) {
}

bool MySQL::connect() {
    // 已经连接了
    if(mysql_ && !hasError_) {
        return true;
    }
    MYSQL* mysql = mysql_init(params_, 0);
    if(!mysql) {
        ERROR("mysql connect failed");
        hasError_ = true;
        return false;
    }
    hasError_ = false;
    poolSize_ = util::GetParamValue(params_, "pool", 5);
    mysql_.reset(mysql, mysql_close);
    return true;
}

bool MySQL::ping() {
    if(!mysql_) {
        return false;
    }
    // int mysql_ping(MYSQL *mysql)
    // 检查与服务器的连接是否工作，如有必要重新连接。如果与服务器的连接有效返回0。如果出现错误，返回非0值
    if(mysql_ping(mysql_.get())) {
        ERROR("mysql ping failed");
        hasError_ = true;
        return false;
    }
    hasError_ = false;
    return true;
}

void MySQL::mysqlInfo() {
    // mysql_get_client_info() shows the MySQL client version.
    std::cout << "MySQL client version " << 
                    mysql_get_client_info() << std::endl;
}


int MySQL::execute(const std::string& sql) {
    cmd_ = sql;
    // int mysql_query(MYSQL *mysql, const char *query)
    // 执行由“Null终结的字符串”查询指向的SQL查询。
    int r = ::mysql_query(mysql_.get(), cmd_.c_str());
    if(r) {
        ERROR("cmd = {}, error = {}", cmd(), getErrStr());
        hasError_ = true;
    } else {
        hasError_ = false;
    }
    return r;
}

int64_t MySQL::getLastInsertId() {
    // my_ulonglong mysql_insert_id(MYSQL *mysql)
    // 返回上一步 insert 操作产生的 id。The function only works if we have defined an AUTO_INCREMENT column in the table.
    return mysql_insert_id(mysql_.get());
}

//https://stackoverflow.com/questions/4438886/how-to-manage-shared-ptr-that-points-to-internal-data-of-already-referenced-obje
// todo 这里设计的目的
std::shared_ptr<MySQL> MySQL::getMySQL() {
    return MySQL::ptr(this, util::nop<MySQL>);
}

uint64_t MySQL::getAffectedRows() {
    if(!mysql_) {
        return 0;
    }
    return mysql_affected_rows(mysql_.get());
}

std::shared_ptr<MySQLRes> MySQL::query(const std::string& sql) {
    cmd_ = sql;
    MYSQL_RES* res = my_mysql_query(mysql_.get(), cmd_.c_str());
    if(!res) {
        ERROR("query failed");
        hasError_ = true;
        return nullptr;
    }
    hasError_ = false;
    MySQLRes::ptr rt = std::make_shared<MySQLRes>(res, 
                mysql_errno(mysql_.get()), mysql_error(mysql_.get()));
    return rt;
}

std::shared_ptr<MySQLStmt> MySQL::prepare(const std::string& sql) {
    return MySQLStmt::Create(shared_from_this(), sql);
}

// MySQLTransaction::ptr MySQL::openTransaction(bool auto_commit) {
//     return MySQLTransaction::Create(shared_from_this(), auto_commit);
// }


bool MySQL::use(const std::string& dbname)  {
    if(!mysql_) {
        ERROR("mysql null");
        return false;
    }
    if(dbname_ == dbname) {
        return true;
    }
    // int mysql_select_db(MYSQL *mysql, const char *db)
    // 选择数据库。
    if(mysql_select_db(mysql_.get(), dbname.c_str()) == 0) {
        dbname_ = dbname;
        hasError_ = false;
        return true;
    } else {
        ERROR("can't select database");
        dbname_ = "";
        hasError_ = true;
        return false;
    }
}

int MySQL::getErrno() {
    if(!mysql_) {
        return -1;
    }
    // unsigned int mysql_errno(MYSQL *mysql)
    // https://www.docs4dev.com/docs/zh/mysql/5.7/reference/mysql-errno.html
    return mysql_errno(mysql_.get());
}

std::string MySQL::getErrStr() {
    if(!mysql_) {
        return "mysql is null";
    }
    const char* errStr = mysql_error(mysql_.get());
    if(errStr) {
        return errStr;
    }
    return "";
}

uint64_t MySQL::getInsertId() {
    if(mysql_) {
        return mysql_insert_id(mysql_.get());
    }
    return 0;
}

// todo why need this
bool MySQL::isNeedCheck() {
    if(timeDifference(Timestamp::now(), lastUsedTime_) < 5 && !hasError_) {
        return false;
    }
    return true;
}




