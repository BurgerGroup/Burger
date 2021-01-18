#include "log.h"

using namespace burger;

LogFormatter::LogFormatter(const std::string& pattern) 
            : pattern_(pattern) {
}

std::string LogFormatter::format(LogEvent::ptr event) {
    std::stringstream ss;
    for(auto& item : items_) {
        item->format(ss, event);
    }
    return ss.str();
}

void LogFormatter::init() {
    
}

Logger::Logger(const std::string& name) 
    : name_(name) {
}

void Logger::log(LogLevel level, LogEvent::ptr event) {
    if(level >= level_) {
        for(auto& i : appenders_) {
            i->log(level, event);
        }
    }
}

void Logger::debug(LogEvent::ptr event)  {
    log(LogLevel::DEBUG, event);
}

void Logger::info(LogEvent::ptr event) {
    log(LogLevel::INFO, event);
}

void Logger::warn(LogEvent::ptr event) {
    log(LogLevel::WARN, event);
}

void Logger::error(LogEvent::ptr event) {
    log(LogLevel::ERROR, event);
}

void Logger::fatal(LogEvent::ptr event) {
    log(LogLevel::FATAL, event);
}

void Logger::addAppender(LogAppender::ptr appender) {
    appenders_.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
    for(auto it = appenders_.begin();
            it!= appenders_.end(); it++) {
        if(*it == appender) {
            appenders_.erase(it);
            break;
        }
    }
}

void StdoutLogAppender::log(LogLevel level, LogEvent::ptr event) {
    if(level >= level_) {
        std::cout << formatter_->format(event);
    }
}

FileLogAppender::FileLogAppender(const std::string& filename) 
                : filename_(filename) {
}

void FileLogAppender::log(LogLevel level, LogEvent::ptr event) {
    if(level >= level_) {
        filestream_ << formatter_->format(event);
    }
}

bool FileLogAppender::reopen() {
    if(filestream_) {
        filestream_.close(); 
    }
    filestream_.open(filename_);
    return !!filestream_;   // !!
}



