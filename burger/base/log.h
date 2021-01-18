#ifndef LOG_H
#define LOG_H

#include <string>
#include <thread>
#include <memory>
#include <list>
#include <vector>
#include <fstream>
#include <sstream>
#include "Timestamp.h"

namespace burger {
 
// 日志事件
class LogEvent {
public:
    using ptr = std::shared_ptr<LogEvent>;
    LogEvent();
    const std::string& getFile() const { return file_; }
    int getLine() const { return line_; }
    std::thread::id getThreadId() const { return threadId_; }
    const std::string& getContent() const { return content_; }
    const Timestamp& getTime() const { return time_; }
    int64_t getElapsed() const { return elapsed_; } 
private:
    const std::string file_ = nullptr;  // 文件名
    int line_ = 0;                      // 行号
    std::thread::id threadId_;          // 线程id
    std::string content_;               // 
    Timestamp time_;                    // 时间戳
    int64_t elapsed_;                   // 程序启动开始到现在的毫秒数
};

// 日志级别
enum class LogLevel {
    UNKNOWN = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

// 日志格式
class LogFormatter {
public:
    using ptr = std::shared_ptr<LogFormatter>;
    LogFormatter(const std::string& pattern);
    std::string format(LogEvent::ptr event);
private:
    class FormatItem {
    public:
        using ptr = std::shared_ptr<FormatItem>;
        virtual ~FormatItem();
        virtual void format(std::ostream& os , LogEvent::ptr event) = 0;
    };
    void init();
private:
    std::string pattern_;
    std::vector<FormatItem::ptr> items_;
};

//  日志输出地
class LogAppender {
public:
    using ptr = std::shared_ptr<LogAppender>;
    virtual ~LogAppender();
    virtual void log(LogLevel level, LogEvent::ptr event) = 0;
    void setFormatter(LogFormatter::ptr formatter) { formatter_ = }
protected:
    LogLevel level_;
    LogFormatter::ptr formatter_;
};

// 日志输入器
// 定义日志类别
class Logger {
public:
    using ptr = std::shared_ptr<Logger>;
    Logger(const std::string& name = "root");
    void log(LogLevel level, LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    LogLevel getLevel() const { return level_; }
    void setLevel(LogLevel level) { level_ = level; } 

private:
    std::string name_;
    LogLevel level_;
    std::list<LogAppender::ptr> appenders_;   // Appenders set
};

// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender {
public:
    using ptr = std::shared_ptr<StdoutLogAppender>;
    void log(LogLevel level, LogEvent::ptr event) override;
};

// 输出到文件的Appender
class FileLogAppender : public LogAppender {
public:
    using ptr = std::shared_ptr<FileLogAppender>;
    FileLogAppender(const std::string& filename);
    void log(LogLevel level, LogEvent::ptr event) override;
    bool reopen();
private:
    std::string filename_;
    std::ofstream filestream_;
};



} // namespace burge 





 #endif // LOG_H