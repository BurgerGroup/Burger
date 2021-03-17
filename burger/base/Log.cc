#include "Log.h"

using namespace burger;

Logger& Logger::Instance() {
    static Logger log;
    return log;
}

bool Logger::init(const std::string& loggerName, 
        const std::string& filePath, 
        spdlog::level::level_enum level) {
    if(isInited_) return true;
    try {
        // check log path and try to create log directory
        // 这里需要文件操作吗?
        // fs::path log_path(filePath);
        // fs::path log_dir = log_path.parent_path();
        // if (!fs::exists(log_path)) {
        //     fs::create_directories(log_dir);
        // }
        // initialize spdlog
        constexpr std::size_t log_buffer_size = 32 * 1024; // 32kb
        spdlog::init_thread_pool(log_buffer_size, std::thread::hardware_concurrency());
        // spdlog::set_level(level);  // todo 这里设置无法影响道logger 
        // spdlog::set_pattern("%Y-%m-%d %H:%M:%S [%l] [tid : %t] [%s : %# <%!>] %v");
        spdlog::flush_every(std::chrono::seconds(3));  // todo 这里能影响道logger吗
        // spdlog::flush_on(spdlog::level::warn);

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt> (filePath, 1024*1024*5, 5, false);
        // auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath, true);
        // Async : https://github.com/gabime/spdlog/wiki/6.-Asynchronous-logging
        // std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
        std::vector<spdlog::sink_ptr> sinks{file_sink};   // 暂时先不要输出到显示屏
        std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>(loggerName,  sinks.begin(), sinks.end());
        logger->set_level(level);    // 需要单独设置logger的level      
        logger->set_pattern("%Y-%m-%d %H:%M:%S [%l] [tid : %t] [%s : %# <%!>] %v");
        logger->flush_on(spdlog::level::warn);
        spdlog::set_default_logger(logger);
    } catch(const spdlog::spdlog_ex& ex) {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }
    isInited_ = true;
    return true;
}

void Logger::setLevel(spdlog::level::level_enum level) {
    spdlog::set_level(level);  // 后面setlevel就可以影响logger了
}


