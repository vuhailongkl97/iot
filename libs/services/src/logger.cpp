#include "logger.h"

static std::once_flag called;
static std::shared_ptr<spdlog::logger> m_logger;


void spdLogger::initialize()
{
    auto max_size = 1048576 * 3;
    auto max_files = 3;
    auto logger = spdlog::rotating_logger_mt("iot", "/tmp/rotating.txt", max_size, max_files);

    spdlog::set_pattern("[%H:%M:%S] %v");
    spdlog::flush_every(std::chrono::seconds(10));
    m_logger = logger;
}

std::shared_ptr<spdlog::logger> spdLogger::getLoggerInstance()
{
    std::call_once(called, initializeLog);
    return m_logger;
}

Logger &spdLogger::getInstance() {
    static spdLogger m_logger;
    return m_logger;
}

void spdLogger::info(const char *str) {
    getLoggerInstance()->info("{}", str);
}
void spdLogger::error(const char *str) {
    getLoggerInstance()->info("{}", str);
}
