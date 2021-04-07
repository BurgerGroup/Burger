#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <memory>
#include "Singleton.h"
#include "ini/ini.h"
#include "Env.h"
#include "util.h"
#include "Log.h"

namespace burger {
    
// todo : 类型 -- 泛型
// todo:  getSize 优化
// todo : 更加清爽的调用
class Config {
public:
    static Config& Instance();

    // bool init(const std::string fileName = "./conf.ini");
    int getInt(const std::string& section, const std::string& search, int defaultVal = 0);
    size_t getSize(const std::string& section, const std::string& search, int defaultVal = 0);
    std::string getString(const std::string& section, const std::string& search, const std::string& defaultVal = "");
    bool getBool(const std::string& section, const std::string& search, bool defaultVal = true);
    double getDouble(const std::string& section, const std::string& search, double defaultVal = 0.0);
private:
    Config();
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    bool isInited_{false};
    std::unique_ptr<INIReader> reader_;
};

} // namespace burger

// #define Conf Config::Instance();


#endif // CONFIG_H