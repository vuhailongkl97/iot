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

spdLogger::~spdLogger() {}

void spdLogger::initialize(std::shared_ptr<impl>& _impl)
{
    if (_impl == nullptr) { _impl = std::make_shared<impl>(); }
    auto max_size = 1048576 * 3;
    auto max_files = 3;
    auto logger = spdlog::rotating_logger_mt("iot", "/tmp/rotating.txt",
                                             max_size, max_files);

    spdlog::set_pattern("[%H:%M:%S] [%^---%L---%$] %v");
    spdlog::flush_every(std::chrono::minutes(2));
    _impl->m_logger = logger;
}

std::shared_ptr<spdLogger::impl> spdLogger::getLoggerInstance()
{
    std::call_once(called, initialize, m_impl);
    return m_impl;
}

Logger& spdLogger::getInstance()
{
    static spdLogger _loggerInst;
    return _loggerInst;
}

void spdLogger::info(const char* str) { getLoggerInstance()->info("{}", str); }

void spdLogger::error(const char* str)
{
    getLoggerInstance()->error("{}", str);
}

void spdLogger::debug(const char* str)
{
    getLoggerInstance()->debug("{}", str);
}

void spdLogger::warn(const char* str) { getLoggerInstance()->warn("{}", str); }
