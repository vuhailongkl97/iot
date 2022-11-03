#pragma once
#undef SPDLOG_HEADER_ONLY
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

enum class LOGLV
{
    INFO,
    WARNING,
    DEBUG,
    ERROR
};
class Logger
{
protected:
    LOGLV m_loglevel = LOGLV::INFO;

public:
    virtual void info(const char* str) = 0;
    virtual void error(const char* str) = 0;
    virtual void debug(const char* str) = 0;
    virtual void warn(const char* str) = 0;
    virtual Logger& getInstance() = 0;
};

class spdLogger final : public Logger
{
private:
    static void initialize();
public:
    void info(const char* str) override;
    void error(const char* str) override;
    void debug(const char* str) override;
    void warn(const char* str) override;
    Logger& getInstance() override;
    std::shared_ptr<spdlog::logger> getLoggerInstance();
};
