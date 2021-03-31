#include <benchmark/benchmark.h>
#include <iostream>
#include <string>
#include <chrono>
#include <memory>
#include <thread>
#include <libgen.h>

// #include "burger/base/Log.h"
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE//
const int QUEUE_SIZE = 8192;

using namespace std;
using namespace burger;

/************************** spdlog *************************/
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/async.h"

std::shared_ptr<spdlog::logger> initSpdLogger(bool isAsync, bool isToFile, bool isMultiThread = false) {
    if(isAsync) {
        spdlog::init_thread_pool(QUEUE_SIZE, std::thread::hardware_concurrency());
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

    if(isAsync) logger->flush();
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
    spdlog::drop(logger->name());
}

//********************************************
//boost
//********************************************
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/core.hpp> 
#include <boost/log/expressions.hpp> 
#include <boost/log/sinks/text_ostream_backend.hpp> 
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/severity_channel_logger.hpp> 
#include <boost/log/sources/record_ostream.hpp> 
#include <boost/log/trivial.hpp> 
#include <boost/log/support/date_time.hpp> 
#include <boost/log/utility/setup/common_attributes.hpp> 
#include <boost/log/sources/severity_logger.hpp> 
#include <boost/log/attributes/current_thread_id.hpp> 
#include <boost/log/attributes/current_process_name.hpp> 
#include <boost/log/attributes/attribute.hpp> 
#include <boost/log/attributes/attribute_cast.hpp> 
#include <boost/log/attributes/attribute_value.hpp> 

#define BOOST_LOG_DYN_LINK 1
#include <boost/log/common.hpp>  
#include <boost/log/expressions.hpp>  
  
  
#include <boost/log/utility/setup/file.hpp>  
#include <boost/log/utility/setup/console.hpp>  
#include <boost/log/utility/setup/common_attributes.hpp>  
  
  
#include <boost/log/attributes/timer.hpp>  
#include <boost/log/attributes/named_scope.hpp>  
  
  
#include <boost/log/sources/logger.hpp>  
  
  
#include <boost/log/support/date_time.hpp> 
// Related headers 
#include <boost/log/sinks/unbounded_fifo_queue.hpp> 
#include <boost/log/sinks/unbounded_ordering_queue.hpp> 
#include <boost/log/sinks/bounded_fifo_queue.hpp> 
#include <boost/log/sinks/bounded_ordering_queue.hpp> 
#include <boost/log/sinks/drop_on_overflow.hpp> 
#include <boost/log/sinks/block_on_overflow.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace attrs = boost::log::attributes;

enum severity_level { normal, info, warning, error, critical };
std::ostream &operator<<(std::ostream &strm, severity_level level) {
    static const char *strings[] = { "normal", "info", "warning", "error", "critical" };
    if (static_cast<std::size_t>(level) < sizeof(strings) / sizeof(*strings))
        strm << strings[level];
    else
        strm << static_cast<int>(level); return strm;
}


// class BoostLoggerInit { 
//     typedef sinks::asynchronous_sink<sinks::text_ostream_backend, sinks::bounded_fifo_queue<QUEUE_SIZE, sinks::block_on_overflow>> asyn_sink_t;
//     typedef sinks::synchronous_sink<sinks::text_ostream_backend> sync_sink_t;

// public:
//     BoostLoggerInit(bool isAsync, bool isToFile)
//     {
//         if (isAsync) {
//             asyn_sink_ = init_logging<asyn_sink_t>(isToFile);
//         }
//         else {
//             sync_sink_ = init_logging<sync_sink_t>(isToFile);
//         }
//     }

//     ~BoostLoggerInit()
//     {
//         stop_logging(asyn_sink_);
//         stop_logging(sync_sink_);
//     }

//     void Flush()
//     {
//         if (asyn_sink_) asyn_sink_->flush();
//         if (sync_sink_) sync_sink_->flush();
//     }
// private:
//     template<typename SinkType>
//     boost::shared_ptr<SinkType> init_logging(bool isToFile) {
//         logging::add_common_attributes();
//         boost::shared_ptr<logging::core> core = logging::core::get();
//         boost::shared_ptr<SinkType> sink();
//         if (isToFile) { 
//             auto backend = boost::make_shared<sinks::text_file_backend>(
//                             "logs/benchmark_boostlog.log",  keywords::auto_flush = false,
//                             keywords::filter = expr::attr< severity_level >("Severity") >= info);
//             sink = boost::shared_ptr<SinkType>(new SinkType(backend));
//         }
//         else {
//             boost::shared_ptr<sinks::text_ostream_backend> backend = boost::make_shared<sinks::text_ostream_backend>();
//             backend->add_stream(boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));
//             sink = boost::shared_ptr<SinkType>(new SinkType(backend));
//         }
       
//         core->add_sink(sink);
//         sink->set_filter(expr::attr<severity_level>("Severity") >= info);
//         sink->set_formatter(
//             expr::stream  
//             << "["<< expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d, %H:%M:%S.%f")
//             << "] [" << expr::attr< severity_level >("Severity")  
//             << "] <" << __FILE__ << ":" << __LINE__   
//             << "> " << expr::message);
//         return sink;
//     }

//     void stop_logging(boost::shared_ptr<asyn_sink_t> &sink) {
//         if (!sink) {
//             return;
//         }
//         boost::shared_ptr<logging::core> core = logging::core::get();
//         // Remove the sink from the core, so that no records are passed to it 
//         core->remove_sink(sink);
//         // Break the feeding loop 
//         sink->stop();
//         // Flush all log records that may have left buffered 
//         sink->flush();
//         sink.reset();
//     }

//     void stop_logging(boost::shared_ptr<sync_sink_t> &sink) {
//         if (!sink) {
//             return;
//         }
//         boost::shared_ptr<logging::core> core = logging::core::get();
//         // Remove the sink from the core, so that no records are passed to it 
//         core->remove_sink(sink);
//         // Break the feeding loop 
//         //sink->stop();
//         // Flush all log records that may have left buffered 
//         sink->flush();
//         sink.reset();
//     }

//     boost::shared_ptr<asyn_sink_t> asyn_sink_;
//     boost::shared_ptr<sync_sink_t> sync_sink_;
// };
class BoostLoggerInit
{
    typedef sinks::asynchronous_sink<sinks::text_ostream_backend, sinks::bounded_fifo_queue<QUEUE_SIZE, sinks::block_on_overflow>> asyn_sink_t;
    typedef sinks::synchronous_sink<sinks::text_ostream_backend> sync_sink_t;

public:
    BoostLoggerInit(bool async, bool file)
    {
        if (async) {
            _asyn_sink = init_logging<asyn_sink_t>(file);
        }
        else {
            _sync_sink = init_logging<sync_sink_t>(file);
        }
    }

    ~BoostLoggerInit()
    {
        stop_logging(_asyn_sink);
        stop_logging(_sync_sink);
    }

    void Flush()
    {
        if (_asyn_sink) _asyn_sink->flush();
        if (_sync_sink) _sync_sink->flush();
    }
private:
    template<typename SinkType>
    boost::shared_ptr<SinkType> init_logging(bool file) {
        logging::add_common_attributes();
        boost::shared_ptr<logging::core> core = logging::core::get();
        boost::shared_ptr<sinks::text_ostream_backend> backend = boost::make_shared<sinks::text_ostream_backend>();
        if (file)
            backend->add_stream(boost::make_shared<std::ofstream>("boostlog.txt"));
        else
            backend->add_stream(boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));
        boost::shared_ptr<SinkType> sink(new SinkType(backend));
        core->add_sink(sink);
        sink->set_filter(expr::attr<severity_level>("Severity") >= info);
        std::string name = "console_logger";
        if (file) name = "file_logger";
        sink->set_formatter(
            expr::stream
            << "[" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
            << "] [" << name
            << "] [" << expr::attr<severity_level>("Severity")
            << "] "
            << expr::smessage);
        return sink;
    }

    void stop_logging(boost::shared_ptr<asyn_sink_t> &sink) {
        if (!sink) {
            return;
        }
        boost::shared_ptr<logging::core> core = logging::core::get();
        // Remove the sink from the core, so that no records are passed to it 
        core->remove_sink(sink);
        // Break the feeding loop 
        sink->stop();
        // Flush all log records that may have left buffered 
        sink->flush();
        sink.reset();
    }

    void stop_logging(boost::shared_ptr<sync_sink_t> &sink) {
        if (!sink) {
            return;
        }
        boost::shared_ptr<logging::core> core = logging::core::get();
        // Remove the sink from the core, so that no records are passed to it 
        core->remove_sink(sink);
        // Break the feeding loop 
        //sink->stop();
        // Flush all log records that may have left buffered 
        sink->flush();
        sink.reset();
    }

    boost::shared_ptr<asyn_sink_t> _asyn_sink;
    boost::shared_ptr<sync_sink_t> _sync_sink;
};


auto BM_boostlog = [](benchmark::State& state, bool asyn, bool file, bool isMultiThread = false) {
    BoostLoggerInit sink(asyn, file);
    std::string msg(state.range(0), 'b');

    if(isMultiThread) {
        src::severity_logger_mt<severity_level> lg;
        for (auto _ : state) {
            BOOST_LOG_SEV(lg, info) << msg;
        }
        sink.Flush();
        state.SetItemsProcessed(state.iterations());
    }
    else {
        src::severity_logger<severity_level> lg;
        for (auto _ : state) {
            BOOST_LOG_SEV(lg, info) << msg;
        }
        sink.Flush();
        state.SetItemsProcessed(state.iterations());
    }
};


int main(int argc, char** argv) {
// Register the function as a benchmark
    // spdlog
    auto inputs_spdlog = { std::make_tuple("spd_a_c", true, false),
        std::make_tuple("spd_a_f", true, true),
        std::make_tuple("spd_s_c", false, false),
            std::make_tuple("spd_s_f", false, true) };
    for (auto& input : inputs_spdlog)
        benchmark::RegisterBenchmark(std::get<0>(input), BM_spdlog, std::get<1>(input), std::get<2>(input))->Iterations(50000)->Arg(32)->Arg(512);
    // boostlog
    auto inputs_boostlog = { std::make_tuple("boost_a_c", true, false),
        std::make_tuple("boost_a_f", true, true),
        std::make_tuple("boost_s_c", false, false),
            std::make_tuple("boost_s_f", false, true) };
    for (auto& input : inputs_boostlog)
        benchmark::RegisterBenchmark(std::get<0>(input), BM_boostlog, std::get<1>(input), std::get<2>(input))->Iterations(50000)->Arg(32)->Arg(512);
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}