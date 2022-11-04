#pragma once
#undef SPDLOG_HEADER_ONLY

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
};

class spdLogger final : public Logger
{
public:
    struct impl;

public:
    static void initialize(impl&);
    void info(const char* str) override;
    void error(const char* str) override;
    void debug(const char* str) override;
    void warn(const char* str) override;
    static Logger& getInstance();
    impl* getLoggerInstance();
};
