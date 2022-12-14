#pragma once

#include <atomic>
#include <string>
#include <config_mgr.h>
#include <logger.h>
#include <memory>

class Logger;
struct req_impl;
struct res_impl;
using handler_t = res_impl (*)(Config&, req_impl);

enum class NOTIFY_TYPE
{
    DETECT_RET,
    MSG
};

class Interface
{
protected:
    Config& cfg;
    Logger& logger;
    handler_t m_handler;

public:
    Interface(Config& c, Logger& l) : cfg(c), logger(l) {}
    handler_t getHandler() { return m_handler; }
    virtual bool initialize() = 0;
    virtual void run() = 0;
    virtual void notify(NOTIFY_TYPE, std::string content) = 0;
    virtual ~Interface() {}
};

class CrowServer final: public Interface
{
private:
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    CrowServer(Config& c, Logger& l);
    bool initialize() override;
    void run() override;
    void notify(NOTIFY_TYPE, std::string) override;
    ~CrowServer();
    static Interface& getInstance(Config&, Logger&);
};
