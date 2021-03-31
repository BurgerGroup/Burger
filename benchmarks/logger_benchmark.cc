#include <benchmark/benchmark.h>
#include <iostream>
#include <string>

#include "burger/base/Log.h"

using namespace std;
using namespace burger;

/************************** spdlog *************************/
void initSpdLogger()
{
    bool res = Logger::Instance().init("log", "logs/benchmark_spdlog.log", spdlog::level::trace);
    if (!res) {
            ERROR("Logger init error");
	    } else {
            TRACE("Logger setup");
        }
}

static void BM_spdlog(benchmark::State& state) {
    initSpdLogger();
    string msg(state.range(0), 's');
    for (auto _ : state)
        INFO(msg);

    // cout << state.iterations() << endl;
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}

/************************** boost::log *************************/
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

void initBoostLogger() {
    logging::add_file_log  
        (  
            "logs/benchmark_boostlog.log",  keywords::auto_flush = false,
            keywords::filter = expr::attr< severity_level >("Severity") >= info,  
            keywords::format = expr::stream  
                << "["<< expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d, %H:%M:%S.%f")
                << "] [" << expr::attr< severity_level >("Severity")  
                << "] <" << __FILE__ << ":" << __LINE__   
                << "> " << expr::message  
        );  

    logging::add_common_attributes();  
    logging::core::get()->add_global_attribute("Scope", attrs::named_scope());  
    BOOST_LOG_FUNCTION();
}

static void BM_boostlog(benchmark::State& state) {
    initBoostLogger();
    src::severity_logger_mt<severity_level> lg;
    string msg(state.range(0), 'b');
    for (auto _ : state)
        BOOST_LOG_SEV(lg, info) << msg;
    // cout << state.iterations() << endl;
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}

// Register the function as a benchmark
BENCHMARK(BM_spdlog)->Iterations(10000)->Arg(32); 
BENCHMARK(BM_boostlog)->Iterations(10000)->Arg(32);
BENCHMARK(BM_spdlog)->Iterations(50000)->Arg(32); 
BENCHMARK(BM_boostlog)->Iterations(50000)->Arg(32);
BENCHMARK(BM_spdlog)->Iterations(10000)->Arg(512); 
BENCHMARK(BM_boostlog)->Iterations(10000)->Arg(512); 
BENCHMARK_MAIN(); //程序入口