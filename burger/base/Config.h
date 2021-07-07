#ifndef CONFIG_H
#define CONFIG_H


#include <string>
#include <memory>
#include <stack>
#include <map>
#include <queue>
#include <algorithm>
#include "Singleton.h"
#include "ini/ini.h"
#include "Env.h"
#include "Util.h"
#include "Log.h"

namespace burger {

namespace detail {

bool isDigit(char ch);
int getIntFromStringExpression(const std::string&);
int getPriority(const char& ch);
bool isOperator(const std::string& str);
int calculate(int left, int right, const std::string& op);
// std::string getFilePath();

} // namespace compute
    
// todo : 类型 -- 泛型
// todo:  getSize 优化
// todo : 更加清爽的调用
class Config {
public:
    static Config& Instance(const std::string& relativePath = "/.burger/config/conf.ini");

    uint16_t getUInt16(const std::string& section, const std::string& search, int defaultVal = 0);
    int getInt(const std::string& section, const std::string& search, int defaultVal = 0);
    size_t getSize(const std::string& section, const std::string& search, int defaultVal = 0);
    std::string getString(const std::string& section, const std::string& search, const std::string& defaultVal = "");
    bool getBool(const std::string& section, const std::string& search, bool defaultVal = true);
    double getDouble(const std::string& section, const std::string& search, double defaultVal = 0.0);
private:
    Config(const std::string& relativePath);
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    bool isInited_{false};
    std::unique_ptr<INIReader> reader_;
    std::string relativePath_;
};

} // namespace burger

// #define Conf Config::Instance();


#endif // CONFIG_H