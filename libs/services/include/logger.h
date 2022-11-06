#pragma once
#include <memory>

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
private:
    struct impl;
    std::shared_ptr<impl> m_impl;
    std::shared_ptr<impl> getLoggerInstance();

public:
    static void initialize(std::shared_ptr<impl>&);
    void info(const char* str) override;
    void error(const char* str) override;
    void debug(const char* str) override;
    void warn(const char* str) override;
    static Logger& getInstance();
    ~spdLogger();
};
