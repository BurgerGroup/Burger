#include "MysqlManager.h"
#include "MysqlTransaction.h"
#include "mysql.h"

using namespace burger;
using namespace burger::db;

MySQLManager::MySQLManager()
    :maxConn_(10) {
    mysql_library_init(0, nullptr, nullptr);
}

MySQLManager::~MySQLManager() {
    mysql_library_end();
    for(auto& i : conns_) {
        for(auto& n : i.second) {
            delete n;
        }
    }
}

MySQL::ptr MySQLManager::get(const std::string& name) {
    // todo 这里可以不手动unlock吗
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = conns_.find(name);
    if(it != conns_.end()) {
        if(!it->second.empty()) {
            MySQL* rt = it->second.front();
            it->second.pop_front();
            lock.unlock();

            if(!rt->isNeedCheck()) {
                rt->lastUsedTime_ = time(0);
                return MySQL::ptr(rt, std::bind(&MySQLManager::freeMySQL,
                            this, name, std::placeholders::_1));
            }
            if(rt->ping()) {
                rt->lastUsedTime_ = time(0);
                return MySQL::ptr(rt, std::bind(&MySQLManager::freeMySQL,
                            this, name, std::placeholders::_1));
            } else if(rt->connect()) {
                rt->lastUsedTime_ = time(0);
                return MySQL::ptr(rt, std::bind(&MySQLManager::freeMySQL,
                            this, name, std::placeholders::_1));
            } else {
                WARN("reconnect {} fail", name);
                return nullptr;
            }
        }
    }
    auto config = g_mysql_dbs->getValue();
    auto sit = config.find(name);
    std::map<std::string, std::string> args;
    if(sit != config.end()) {
        args = sit->second;
    } else {
        sit = dbDefines_.find(name);
        if(sit != dbDefines_.end()) {
            args = sit->second;
        } else {
            return nullptr;
        }
    }
    // todo why here unlock
    lock.unlock();
    MySQL* rt = new MySQL(args);
    if(rt->connect()) {
        rt->lastUsedTime_ = time(0);
        return MySQL::ptr(rt, std::bind(&MySQLManager::freeMySQL,
                    this, name, std::placeholders::_1));
    } else {
        delete rt;
        return nullptr;
    }
}

void MySQLManager::registerMySQL(const std::string& name, const std::map<std::string, std::string>& params) {
    std::lock_guard<std::mutex> lock(mutex_);
    dbDefines_[name] = params;
}

void MySQLManager::checkConnection(int sec) {
    time_t now = time(0);
    std::vector<MySQL*> conns;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for(auto& i : conns_) {
            for(auto it = i.second.begin();
                    it != i.second.end();) {
                if((int)(now - (*it)->lastUsedTime_) >= sec) {
                    auto tmp = *it;
                    i.second.erase(it++);
                    conns.push_back(tmp);
                } else {
                    ++it;
                }
            }
        }
    }
    for(auto& i : conns) {
        delete i;
    }
}

int MySQLManager::execute(const std::string& name, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    int rt = execute(name, format, ap);
    va_end(ap);
    return rt;
}

int MySQLManager::execute(const std::string& name, const char* format, va_list ap) {
    auto conn = get(name);
    if(!conn) {
        ERROR("MySQLManager::execute, get({}) fail, format= {}", name, format);
        return -1;
    }
    return conn->execute(format, ap);
}

int MySQLManager::execute(const std::string& name, const std::string& sql) {
    auto conn = get(name);
    if(!conn) {
        ERROR("MySQLManager::execute, get({}) fail, sql= {}", name, sql);
        return -1;
    }
    return conn->execute(sql);
}

ISQLData::ptr MySQLManager::query(const std::string& name, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    auto res = query(name, format, ap);
    va_end(ap);
    return res;
}

ISQLData::ptr MySQLManager::query(const std::string& name, const char* format, va_list ap) {
    auto conn = get(name);
    if(!conn) {
        ERROR("MySQLManager::query, get({}) fail, format=", name, format);
        return nullptr;
    }
    return conn->query(format, ap);
}

ISQLData::ptr MySQLManager::query(const std::string& name, const std::string& sql) {
    auto conn = get(name);
    if(!conn) {
        ERROR("MySQLManager::query, get({}) fail, sql= {}", name, sql);
        return nullptr;
    }
    return conn->query(sql);
}

MySQLTransaction::ptr MySQLManager::openTransaction(const std::string& name, bool auto_commit) {
    auto conn = get(name);
    if(!conn) {
        ERROR("MySQLManager::openTransaction, get({}) fail", name);
        return nullptr;
    }
    MySQLTransaction::ptr trans(MySQLTransaction::Create(conn, auto_commit));
    return trans;
}

void MySQLManager::freeMySQL(const std::string& name, MySQL* m) {
    if(m->mysql_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if(conns_[name].size() < (size_t)m->poolSize) {
            conns_[name].push_back(m);
            return;
        }
    }
    delete m;
}