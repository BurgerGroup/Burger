#include "mysql.h"

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
} // namespace 

MySQL::MySQL(const std::map<std::string, std::string>& args)
    : params_(args),
    lastUsedTime_(),
    hasError_(false),
    poolSize_(10) {
}

bool MySQL::connect() {
    // 已经连接了
    if(mysql_ && !hasError_) {
        return true;
    }
    MYSQL* m = mysql_init(params_, 0);
    if(!m) {
        hasError_ = true;
        return false;
    }
    hasError_ = false;
    poolSize_ = util::GetParamValue(params_, "pool", 5);
    mysql_.reset(m, mysql_close);
    return true;
}

IStmt::ptr MySQL::prepare(const std::string& sql) {
    return MySQLStmt::Create(shared_from_this(), sql);
}

ITransaction::ptr MySQL::openTransaction(bool auto_commit) {
    return MySQLTransaction::Create(shared_from_this(), auto_commit);
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
    const char* str = mysql_error(m_mysql.get());
    if(str) {
        return str;
    }
    return "";
}




