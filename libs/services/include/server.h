#pragma once

/*
 * This task is responsible for changing threshold of the detection via
 * network interface using HTTP
 * */
#include <atomic>
#include <string>
#include <config_mgr.h>
#include <logger.h>
#include <memory>

struct req_impl;
struct res_impl;
using handler_t = res_impl (*)(Config&, req_impl);
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
    virtual void notify(std::string content) = 0;
};

class CrowServer : public Interface
{
public:
    struct impl;
    // temporary use public. it should be private
public:
    std::unique_ptr<impl> pimpl;

public:
    CrowServer(Config& c, Logger& l);
    bool initialize() override;
    void run() override;
    void notify(std::string) override;
};
