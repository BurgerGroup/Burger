#ifndef LOG_H
#define LOG_H

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <iostream>
#include <memory>
#include <boost/noncopyable.hpp>

// spd 带行号的打印，同时输出console和文件
#define DEBUG(...) SPDLOG_LOGGER_DEBUG(spdlog::default_logger_raw(), __VA_ARGS__);SPDLOG_LOGGER_DEBUG(spdlog::get("daily_logger"), __VA_ARGS__)
#define LOG(...) SPDLOG_LOGGER_INFO(spdlog::default_logger_raw(), __VA_ARGS__);SPDLOG_LOGGER_INFO(spdlog::get("daily_logger"), __VA_ARGS__)
#define WARN(...) SPDLOG_LOGGER_WARN(spdlog::default_logger_raw(), __VA_ARGS__);SPDLOG_LOGGER_WARN(spdlog::get("daily_logger"), __VA_ARGS__)
#define ERROR(...) SPDLOG_LOGGER_ERROR(spdlog::default_logger_raw(), __VA_ARGS__);SPDLOG_LOGGER_ERROR(spdlog::get("daily_logger"), __VA_ARGS__)

namespace burger {
namespace log {

class Logger : boost::noncopyable {
public:

private:
    Logger();
    ~Logger();
        
};










// todo : when  spdlog::drop_all(); fix me
inline std::shared_ptr<spdlog::logger> logstart() {
    // 每天2:30 am 新建一个日志文件
    auto logger = spdlog::daily_logger_mt("daily_logger", "logs/daily.txt", 2, 30);
    // 遇到warn flush日志，防止丢失
    logger->flush_on(spdlog::level::warn);
    // Set the default logger to file logger
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);
    spdlog::set_level(spdlog::level::trace); // Set global log level to trace
    spdlog::set_pattern("%Y-%m-%d %H:%M:%S [%l] [tid : %t] [%s : %# <%!>] %v");
    return logger;
}
}
    
} // namespace burge 


#endif // LOG_H