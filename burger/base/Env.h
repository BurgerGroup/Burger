#ifndef ENV_H
#define ENV_H

#include <mutex>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <iomanip> // std::setw
#include "Singleton.h"
#include "RWmutex.h"

namespace burger {

// 系统 -- 环境变量
class Env {
public:
    bool init(int argc, char** argv);

    void add(const std::string& key, const std::string& val);
    bool has(const std::string& key);
    void del(const std::string& key);
    std::string get(const std::string& key, const std::string& default_value = "");

    void addHelp(const std::string& key, const std::string& desc);
    void removeHelp(const std::string& key);
    void printHelp();

    const std::string& getExe() const { return exe_;}
    const std::string& getCwd() const { return cwd_;}

    bool setEnv(const std::string& key, const std::string& val);
    std::string getEnv(const std::string& key, const std::string& default_value = "");

    std::string getAbsolutePath(const std::string& path) const;
    std::string getAbsoluteWorkPath(const std::string& path) const;
    // std::string getConfigPath(); // todo
private:
    RWMutex mutex_;  // todo : 这里读写锁效率测试
    std::map<std::string, std::string> argsMap_;
    std::vector<std::pair<std::string, std::string> > helps_;

    std::string program_;
    std::string exe_;  
    std::string cwd_;
};

using EnvMgr = Singleton<Env>;


} // namespace burger



#endif // ENV_H