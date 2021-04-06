#include "Config.h"

using namespace burger;
Config& Config::Instance() {
    static Config conf;
    return conf;
}

int Config::getInt(const std::string& section, const std::string& search, int defaultVal) {
    return reader_->GetInteger(section, search, defaultVal);
}

std::string Config::getString(const std::string& section, const std::string& search, const std::string& defaultVal) {
    return reader_->Get(section, search, defaultVal);
}

bool Config::getBool(const std::string& section, const std::string& search, bool defaultVal) {
    return reader_->GetBoolean(section, search, defaultVal);
}

double Config::getDouble(const std::string& section, const std::string& search, double defaultVal) {
    return reader_->GetDouble(section, search, defaultVal);
}

// todo: 这种方式不够好，修改灵活点，不要写死,或者写一个setConfigFile()
Config::Config() {

    std::string filePath = "../config/conf.ini";

    reader_ = util::make_unique<INIReader>(filePath);
    if(reader_->ParseError() != 0) {
        ERROR("Can't load config file");
    }
}






