/**
 * 1. 解决了格式问题
 * 2. 解决了set_level无效问题
 * 3. 解决了pattern 无效问题
 * 4. TODO: 为什么spdlog::set_level在init中无效，后面设置有效
 * 5. TODO: 我们这里需要filesystem去处理吗
 * 6. TODO: 我们需要异步日志吗
 * 7. TODO: spdlog::flush_every能影响道logger吗
 * 8. 解决了stdout和file同时的问题 -- sinks
 * 9. TODO: 能否在析构函数里放置shutdown，能否包装起来自动shutdown？
 * 10. TODO: 滚动日志参数如何设置合理
 */

#include "burger/base/Log.h"
using namespace burger;

int main() {
    /* 1.原始的写法

    if (!Logger::Instance().init("logs/test.log", "log", spdlog::level::trace)) {
        //do something...
 	    return 1;
    }
    spdlog::set_level(spdlog::level::info);

    */

    // LOGGER();
    // ONLY_TO_CONSOLE;
    ONLY_TO_FILE;
    LOGGER("logs/marco_test.log", "TEST");
    // LOGGER("logs/marco_test2.log");
    // LOGGER_WITH_NAME("BURGER");
    LOG_LEVEL_DEBUG;
    TRACE("TRACE");
    DEBUG("DEBUG");
    INFO("INFO");
    WARN("WARN");
    DEBUG("3 {} ", 1);
    CRITICAL("CRITICAL");
    // Logger::shutdown();  // when do this
}