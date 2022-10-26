#include "logger.h"

static std::once_flag called;
static std::shared_ptr<spdlog::logger> m_logger;


void MLogger::initializeLog()
{
    auto max_size = 1048576 * 10;
    auto max_files = 3;
    auto logger = spdlog::rotating_logger_mt("iot", "logs/rotating.txt", max_size, max_files);

    spdlog::set_pattern("[%H:%M:%S] %v");
    spdlog::flush_every(std::chrono::seconds(10));
    m_logger = logger;
}

std::shared_ptr<spdlog::logger> MLogger::getLoggerInstance()
{
    std::call_once(called, initializeLog);
    return m_logger;
}
