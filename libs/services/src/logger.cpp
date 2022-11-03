#include "logger.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

static std::once_flag called;
static std::shared_ptr<spdlog::logger> m_logger;

struct spdLogger::impl
{
    std::shared_ptr<spdlog::logger> m_logger;

    void info(const char* fmt, const char* str) { m_logger->info(fmt, str); }
    void error(const char* fmt, const char* str) { m_logger->error(fmt, str); }
    void debug(const char* fmt, const char* str) { m_logger->debug(fmt, str); }
    void warn(const char* fmt, const char* str) { m_logger->warn(fmt, str); }
};
void spdLogger::initialize(impl& _impl)
{
    auto max_size = 1048576 * 3;
    auto max_files = 3;
    auto logger = spdlog::rotating_logger_mt("iot", "/tmp/rotating.txt", max_size, max_files);

    spdlog::set_pattern("[%H:%M:%S] [%^---%L---%$] %v");
    spdlog::flush_every(std::chrono::minutes(2));
    _impl.m_logger = logger;
}

spdLogger::impl* spdLogger::getLoggerInstance()
{
    static impl _impl;
    std::call_once(called, initialize, _impl);
    return &_impl;
}

Logger &spdLogger::getInstance() {
    static spdLogger _loggerInst;
    return _loggerInst;
}

void spdLogger::info(const char *str) {
    getLoggerInstance()->info("{}", str);
}

void spdLogger::error(const char *str) {
    getLoggerInstance()->error("{}", str);
}

void spdLogger::debug(const char *str) {
    getLoggerInstance()->debug("{}", str);
}

void spdLogger::warn(const char *str) {
    getLoggerInstance()->warn("{}", str);
}
