#include <benchmark/benchmark.h>
#include <iostream>
#include <string>
#include <memory>

#include "burger/base/Log.h"

using namespace std;
using namespace burger;

// /************************** spdlog *************************/
// void initSpdLogger()
// {
//     bool res = Logger::Instance().init("log", "logs/benchmark_spdlog.log", spdlog::level::trace);
//     if (!res) {
//             ERROR("Logger init error");
// 	    } else {
//             TRACE("Logger setup");
//         }
// }

// static void BM_spdlog(benchmark::State& state) {
//     initSpdLogger();
//     string msg(state.range(0), 's');
//     for (auto _ : state)
//         INFO(msg);

//     // cout << state.iterations() << endl;
//     state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
// }

const int QUEUE_SIZE = 8192;
const int iter_times = 30000;

/************************** spdlog *************************/
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/async.h"

std::shared_ptr<spdlog::logger> initSpdLogger(bool isAsync, bool isToFile, bool isMultiThread = false) {
    if(isAsync) {
        spdlog::init_thread_pool(QUEUE_SIZE, 1/*std::thread::hardware_concurrency()*/);
        if(isToFile) {
            if(isMultiThread) {
                return spdlog::basic_logger_mt<spdlog::async_factory>("file_logger", "logs/spdlog.log", false);
            }
            else {
                return spdlog::basic_logger_st<spdlog::async_factory>("file_logger", "logs/spdlog.log", false);
            }
        }
        else {
            if(isMultiThread) {
                return spdlog::stdout_logger_mt<spdlog::async_factory>("console_logger");
            }
            else {
                return spdlog::stdout_logger_st<spdlog::async_factory>("console_logger");
            }
        }
    }
    else {
        if(isToFile) {
            if(isMultiThread) {
                return spdlog::basic_logger_mt("file_logger", "logs/spdlog.log", false);
            }
            else {
                return spdlog::basic_logger_st("file_logger", "logs/spdlog.log", false);
            }
        }
        else {
            if(isMultiThread) {
                return spdlog::stdout_logger_mt("console_logger");
            }
            else {
                return spdlog::stdout_logger_st("console_logger");
            }
        }
    }
}

auto BM_spdlog = [](benchmark::State& state, bool isAsync, bool isToFile, bool isMultiThread = false)-> void {
    auto logger = initSpdLogger(isAsync, isToFile, isMultiThread);
    logger->set_pattern("%Y-%m-%d %H:%M:%S [%l] [tid : %t] [%s : %# <%!>] %v");
    spdlog::set_default_logger(logger);
    string msg(state.range(0), 's');
    for (auto _ : state){
        SPDLOG_LOGGER_INFO(logger, msg);
    }

    logger->flush();
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
    state.SetItemsProcessed(state.iterations());
    spdlog::drop(logger->name());
};
/************************** boost::log *************************/
#define BOOST_LOG_DYN_LINK 1
#include <fstream>
#include <boost/log/common.hpp>  
#include <boost/log/expressions.hpp>  
  
  
#include <boost/log/utility/setup/file.hpp>  
#include <boost/log/utility/setup/console.hpp>  
#include <boost/log/utility/setup/common_attributes.hpp>  
  
  
#include <boost/log/attributes/timer.hpp>  
#include <boost/log/attributes/named_scope.hpp>  
  
  
#include <boost/log/sources/logger.hpp>  
  
  
#include <boost/log/support/date_time.hpp>  

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/log/core.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include <boost/log/sinks/unbounded_fifo_queue.hpp> 
#include <boost/log/sinks/unbounded_ordering_queue.hpp> 
#include <boost/log/sinks/bounded_fifo_queue.hpp> 
#include <boost/log/sinks/bounded_ordering_queue.hpp> 
#include <boost/log/sinks/drop_on_overflow.hpp> 
#include <boost/log/sinks/block_on_overflow.hpp>

#include <libgen.h>
  
  
namespace logging = boost::log;  
namespace sinks = boost::log::sinks;  
namespace attrs = boost::log::attributes;  
namespace src = boost::log::sources;  
namespace expr = boost::log::expressions;  
namespace keywords = boost::log::keywords;  



enum severity_level { normal, info, warning, error, critical };

std::ostream &operator<<(std::ostream &strm, severity_level level) {
    static const char *strings[] = { "normal", "info", "warning", "error", "critical" };
    if (static_cast<std::size_t>(level) < sizeof(strings) / sizeof(*strings))
        strm << strings[level];
    else
        strm << static_cast<int>(level); return strm;
}

class initBoostLogger {
public:
    typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sync_sink;
    typedef sinks::asynchronous_sink< sinks::text_ostream_backend, sinks::bounded_fifo_queue<QUEUE_SIZE, sinks::block_on_overflow> > text_async_sink;

    initBoostLogger(bool isAsync, bool isToFile)
    {
        if(isAsync) {
            async_sink_ = initLogger<text_async_sink>(isToFile);
        }
        else {
            sync_sink_ = initLogger<text_sync_sink>(isToFile);
        }
    }

    ~initBoostLogger()
    {
        if(async_sink_) {
            logging::core::get()->remove_sink(async_sink_);
            async_sink_->stop();
            async_sink_->flush();
            async_sink_.reset();
        }
        if(sync_sink_) {
            logging::core::get()->remove_sink(sync_sink_);
            // sync_sink_->stop();
            sync_sink_->flush();
            sync_sink_.reset();
        }
    }

    void flush() {
        if(async_sink_) async_sink_->flush();
        if(sync_sink_) sync_sink_->flush();
    }

    template<typename sink_t>
    boost::shared_ptr< sink_t > initLogger(bool isToFile) {
        boost::shared_ptr< sink_t > sink = boost::make_shared< sink_t >();
        if(isToFile) {
            // Add a stream to write log to
            sink->locked_backend()->add_stream(
                boost::make_shared< std::ofstream >("logs/boostlog.log"));
        }
        else {
            sink->locked_backend()->add_stream(
                boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));
        }

        const char* filename = __FILE__;
        const char* slash = strrchr(filename, '/') + 1;
        sink->set_formatter(
                expr::stream  
                << "["<< expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d, %H:%M:%S.%f")
                << "] [" << expr::attr< severity_level >("Severity")  
                << "] <" << slash << ":" << __LINE__   
                << "> " << expr::message);

        // Register the sink in the logging core
        logging::core::get()->add_sink(sink);

        logging::add_common_attributes();  
        logging::core::get()->add_global_attribute("Scope", attrs::named_scope()); 
        logging::core::get()->set_filter(expr::attr<severity_level>("Severity") >= info);
        BOOST_LOG_FUNCTION();

        return sink;
}

private:
    boost::shared_ptr<text_async_sink> async_sink_;
    boost::shared_ptr<text_sync_sink> sync_sink_;
};


auto BM_boostlog = [](benchmark::State& state, bool isAsync, bool isToFile, bool isMultiThread = false) -> void {
    initBoostLogger sink(isAsync, isToFile);
    string msg(state.range(0), 'b');

    if(isMultiThread) {
        src::severity_logger_mt<severity_level> lg;
        for (auto _ : state)
            BOOST_LOG_SEV(lg, info) << msg;
    }
    else {
        src::severity_logger<severity_level> lg;
        for (auto _ : state)
            BOOST_LOG_SEV(lg, info) << msg;
    }
    
    sink.flush();
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
    state.SetItemsProcessed(state.iterations());
};

int main(int argc, char** argv) {
    // spdlog
    auto inputs_spdlog = { std::make_tuple("spd_async_console_st", true, false, false),
                        std::make_tuple("spd_sync_console_st", false, false, false),
                        std::make_tuple("spd_async_console_mt", true, false, true),
                        std::make_tuple("spd_sync_console_mt", false, false, true),
                        std::make_tuple("spd_async_file_st", true, true, false),
                        std::make_tuple("spd_async_file_mt", true, true, true),
                        std::make_tuple("spd_sync_file_st", false, true, false),
                        std::make_tuple("spd_sync_file_mt", false, true, true) };
    for (auto& input : inputs_spdlog)
        benchmark::RegisterBenchmark(std::get<0>(input), BM_spdlog, std::get<1>(input), std::get<2>(input), std::get<3>(input))->Iterations(iter_times)->Arg(32)->Arg(512);
    
    
    // boostlog
    auto inputs_boostlog = { std::make_tuple("boost_async_console_st", true, false, false),
                        std::make_tuple("boost_sync_console_st", false, false, false),
                        std::make_tuple("boost_async_console_mt", true, false, true),
                        std::make_tuple("boost_sync_console_mt", false, false, true),
                        std::make_tuple("boost_async_file_st", true, true, false),
                        std::make_tuple("boost_async_file_mt", true, true, true),
                        std::make_tuple("boost_sync_file_st", false, true, false),
                        std::make_tuple("boost_sync_file_mt", false, true, true) };
    for (auto& input : inputs_boostlog)
        benchmark::RegisterBenchmark(std::get<0>(input), BM_boostlog, std::get<1>(input), std::get<2>(input), std::get<3>(input))->Iterations(iter_times)->Arg(32)->Arg(512);
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}