#include "Log.h"
#include "Util.h"
#include "spdlog/pattern_formatter.h"

using namespace burger;

std::atomic<bool> Logger::writeToConsole_{true};
std::atomic<bool> Logger::writeToFile_{true};

// suppot for coid
class co_formatter_flag : public spdlog::custom_flag_formatter {
public:
    void format(const spdlog::details::log_msg &, const std::tm &, spdlog::memory_buf_t &dest) override {
        std::string coId = std::to_string(util::getCoId());  // todo : 可以优化吗 
        dest.append(coId.data(), coId.data() + coId.size());    
    }

    std::unique_ptr<custom_flag_formatter> clone() const override {
        return spdlog::details::make_unique<co_formatter_flag>();
    }
};


Logger& Logger::Instance() {
    static Logger log;
    return log;
}

bool Logger::init(const std::string& filePath, 
                const std::string& loggerName, 
                spdlog::level::level_enum level) {
    if(isInited_) return true;
    if(!writeToFile_ && !writeToConsole_) {
        std::cout << "Initialized AN EMPTY Logger!" << std::endl;
        return true;
    }
    try {
        // check log path and try to create log directory
        // 这里需要文件操作吗?
        // fs::path log_path(filePath);
        // fs::path log_dir = log_path.parent_path();
        // if (!fs::exists(log_path)) {
        //     fs::create_directories(log_dir);
        // }
        // initialize spdlog
        // constexpr std::size_t log_buffer_size = 32 * 1024; // 32kb
        // spdlog::init_thread_pool(log_buffer_size, std::thread::hardware_concurrency());
        // spdlog::set_level(level);  // todo 这里设置无法影响道logger 
        // spdlog::set_pattern("%Y-%m-%d %H:%M:%S [%l] [tid : %t] [%s : %# <%!>] %v");
        spdlog::flush_every(std::chrono::seconds(3));  // todo 这里能影响道logger吗
        // spdlog::flush_on(spdlog::level::warn);

        // auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath, true);
        // Async : https://github.com/gabime/spdlog/wiki/6.-Asynchronous-logging
        std::vector<spdlog::sink_ptr> sinks;
        if(writeToConsole_) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            sinks.push_back(std::move(console_sink));
        }

        if(writeToFile_) {
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt> (filePath, 1024*1024*5, 5, false);
            sinks.push_back(std::move(file_sink));
        }
        // std::vector<spdlog::sink_ptr> sinks{file_sink};   // 暂时先不要输出到显示屏
        std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>(loggerName,  sinks.begin(), sinks.end());
        logger->set_level(level);    // 需要单独设置logger的level      
        

        auto formatter = util::make_unique<spdlog::pattern_formatter>();
        formatter->add_flag<co_formatter_flag>('*').set_pattern("%Y-%m-%d %H:%M:%S [%l] [tid : %t] [%s : %# <%!>] [coId : %*] %v");
        // logger->set_pattern("%Y-%m-%d %H:%M:%S [%l] [tid : %t] [%s : %# <%!>] %v");
        // spdlog::set_formatter();
        logger->set_formatter(std::move(formatter));
        
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


