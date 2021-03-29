#include "Env.h"
#include "Util.h"
#include "Log.h"
using namespace burger;

// 通过readlink,将软链接的真实路径传出来
// https://man7.org/linux/man-pages/man2/readlinkat.2.html
bool Env::init(int argc, char** argv) {
    std::string linkPath = "/proc/" + std::to_string(::getpid()) + "/exe"; // todo :test here
    std::cout << linkPath << std::endl;
    char path[1024] = {0};
    ssize_t res = readlink(linkPath.c_str(), path, sizeof(path));
    if(res == -1) {
        ERROR("readlink error");
    }
    // /path/xxx/exe
    exe_ = path;
    // 读 /path/xxx
    size_t pos = exe_.find_last_of("/");
    cwd_ = exe_.substr(0, pos) + "/";  

    program_ = argv[0];
    // -config /path/to/config -file xxxx -d
    const char* now_key = nullptr;
    for(int i = 1; i < argc; ++i) {
        if(argv[i][0] == '-') {
            if(strlen(argv[i]) > 1) {
                if(now_key) {
                    add(now_key, "");
                }
                now_key = argv[i] + 1;
            } else {
                ERROR("invalid arg idx = {} val = {}", i , argv[i]);
                return false;
            }
        } else {
            if(now_key) {
                add(now_key, argv[i]);
                now_key = nullptr;
            } else {
                ERROR("invalid arg idx = {} val = {}", i, argv[i]);
                return false;
            }
        }
    }
    if(now_key) {
        add(now_key, "");
    }
    return true;
}

void Env::add(const std::string& key, const std::string& val)  {
    RWMutex::WriteLock lock(mutex_);
    argsMap_[key] = val;
}

bool Env::has(const std::string& key) {
    RWMutex::ReadLock lock(mutex_);
    auto it = argsMap_.find(key);
    return it != argsMap_.end();
}

void Env::del(const std::string& key) {
    RWMutex::WriteLock lock(mutex_);
    argsMap_.erase(key);  // todo erase 不存在的key是否会出问题
}

std::string Env::get(const std::string& key, const std::string& default_value) {
    RWMutex::ReadLock lock(mutex_);
    auto it = argsMap_.find(key);
    return it != argsMap_.end() ? it->second : default_value;
}

void Env::addHelp(const std::string& key, const std::string& desc) {
    removeHelp(key);
    RWMutex::WriteLock lock(mutex_);
    helps_.push_back(std::make_pair(key, desc));
}

void Env::removeHelp(const std::string& key) {
    RWMutex::WriteLock lock(mutex_);
    for(auto it = helps_.begin(); it != helps_.end();) {
        if(it->first == key) {
            it = helps_.erase(it);
        } else {
            ++it;
        }
    }
}

void Env::printHelp() {
    RWMutex::ReadLock lock(mutex_);
    std::cout << "Usage: " << program_ << " [options]" << std::endl;
    for(auto& help : helps_) {
        std::cout << std::setw(5) << "-" << help.first << " : " << help.second << std::endl;
    }
}

bool Env::setEnv(const std::string& key, const std::string& val) {
    // setenv - change or add an environment variable
    // https://man7.org/linux/man-pages/man3/setenv.3.html
    return !setenv(key.c_str(), val.c_str(), 1);
}

std::string Env::getEnv(const std::string& key, const std::string& default_value) {
    const char* v = getenv(key.c_str());
    if(v == nullptr) {
        return default_value;
    }
    return v;
}

std::string Env::getAbsolutePath(const std::string& path) const {
    if(path.empty()) {
        return "/";
    }
    if(path[0] == '/') {
        return path;
    }
    return cwd_ + path;
}

std::string Env::getAbsoluteWorkPath(const std::string& path) const {
    if(path.empty()) {
        return "/";
    }
    if(path[0] == '/') {
        return path;
    }
    // todo:
    // static ConfigVar<std::string>::ptr g_server_work_path =
    //     Config::Lookup<std::string>("server.work_path");
    // return g_server_work_path->getValue() + "/" + path;
    return "";
}











