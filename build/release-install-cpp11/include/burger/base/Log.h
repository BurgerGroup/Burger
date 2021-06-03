#ifndef LOG_H
#define LOG_H
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
// https://github.com/gabime/spdlog/issues/282  Why not use stream syntax
// TODO : 需要异步 ？  
// todo : 比如client不需要在屏幕里显示信息，怎么去除sink?
// todo 系统需要为不同任务设置log?
// TODO : 设计下如何使用
// todo: pattern + coroutine 
// https://spdlog.docsforge.com/v1.x/3.custom-formatting/#extending-spdlog-with-your-own-flags
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
#include <stdlib.h>

namespace burger {

class Logger final : boost::noncopyable {
public:
    static Logger& Instance();
    bool init(const std::string& filePath = "logs/log.txt", 
            const std::string& loggerName = "Logger",
            spdlog::level::level_enum level = spdlog::level::info);

    void setLevel(spdlog::level::level_enum level = spdlog::level::info);
    static void onlyToConsole() { writeToFile_ = false; }
    static void onlyToFile() { writeToConsole_ = false; }
    static void shutdown() { spdlog::shutdown(); };
private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
private:
    std::atomic<bool> isInited_{false};
    static std::atomic<bool> writeToConsole_;
    static std::atomic<bool> writeToFile_;
};

} // namespace burger 


// spd 带行号的打印，同时输出console和文件
#define TRACE(...) SPDLOG_LOGGER_TRACE(spdlog::default_logger_raw(), __VA_ARGS__);
#define DEBUG(...) SPDLOG_LOGGER_DEBUG(spdlog::default_logger_raw(), __VA_ARGS__);
#define INFO(...) SPDLOG_LOGGER_INFO(spdlog::default_logger_raw(), __VA_ARGS__);
#define WARN(...) SPDLOG_LOGGER_WARN(spdlog::default_logger_raw(), __VA_ARGS__);
#define ERROR(...) SPDLOG_LOGGER_ERROR(spdlog::default_logger_raw(), __VA_ARGS__);
#define CRITICAL(...) SPDLOG_LOGGER_CRITICAL(spdlog::default_logger_raw(), __VA_ARGS__);abort();

#define LOG_LEVEL_INFO spdlog::set_level(spdlog::level::info);
#define LOG_LEVEL_DEBUG spdlog::set_level(spdlog::level::debug);
#define LOG_LEVEL_TRACE spdlog::set_level(spdlog::level::trace);
#define LOG_LEVEL_WARN spdlog::set_level(spdlog::level::warn);
#define LOG_LEVEL_ERROR spdlog::set_level(spdlog::level::err);
#define LOG_LEVEL_CRITICAL spdlog::set_level(spdlog::level::critical);

// todo need to improve
// #define LOGGER Logger::Instance().init("Logger", "logs/log.txt", spdlog::level::trace);

#define LOGGER(...) Logger::Instance().init(__VA_ARGS__);
#define LOGGER_WITH_NAME(name) Logger::Instance().init("logs/log.txt", name);
#define ONLY_TO_CONSOLE Logger::onlyToConsole();
#define ONLY_TO_FILE Logger::onlyToFile();
#endif // LOG_H
