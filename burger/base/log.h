#ifndef LOG_H
#define LOG_H
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
// https://github.com/gabime/spdlog/issues/282  Why not use stream syntax
// TODO : 需要异步 ？  
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/basic_file_sink.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/async.h" //support for async logging.

#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <sstream>
#include <boost/noncopyable.hpp>
// #include <boost/filesystem.hpp>
#include <atomic> 
// namespace fs = boost::filesystem;

namespace burger {

class Logger final : boost::noncopyable {
public:
    static Logger& Instance();
    bool init(const std::string& loggerName = "Logger", 
        const std::string& filePath = "logs/test.txt", 
        spdlog::level::level_enum level = spdlog::level::debug);
    void setLevel(spdlog::level::level_enum level = spdlog::level::info);
    static void shutdown() { spdlog::shutdown(); };
private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
private:
    std::atomic<bool> isInited_{false};
};
    

} // namespace burge 


// spd 带行号的打印，同时输出console和文件
#define TRACE(...) SPDLOG_LOGGER_TRACE(spdlog::default_logger_raw(), __VA_ARGS__);
#define DEBUG(...) SPDLOG_LOGGER_DEBUG(spdlog::default_logger_raw(), __VA_ARGS__);
#define INFO(...) SPDLOG_LOGGER_INFO(spdlog::default_logger_raw(), __VA_ARGS__);
#define WARN(...) SPDLOG_LOGGER_WARN(spdlog::default_logger_raw(), __VA_ARGS__);
#define ERROR(...) SPDLOG_LOGGER_ERROR(spdlog::default_logger_raw(), __VA_ARGS__);
#define CRITICAL(...) SPDLOG_LOGGER_CRITICAL(spdlog::default_logger_raw(), __VA_ARGS__);

#endif // LOG_H