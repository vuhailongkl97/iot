#pragma once
#include <string>
#include <vector>
//#include <thread>
#include <logger.h>
#include <config_mgr.h>
#include <server.h>

enum class Jetsonhardware : int
{
    jA0 = 0,
    jCPU,
    jGPU,
    jEND
};

class HardwareManager
{
protected:
    Logger& l;
    Config& cfg;
    Interface& inf;
    std::vector<float> m_temp_thresholds;
    std::string m_name;

public:
    HardwareManager(std::string name, std::vector<float> thresholds, Logger& _l,
                    Config& c, Interface& i) :
      l(_l),
      cfg(c), inf(i), m_temp_thresholds(thresholds), m_name(name)
    {}
    virtual int getAvailableMem() = 0;
    virtual std::vector<float> getTemperatures() = 0;

    void prepareLog();
    void monitor();
};

class Jetson : public HardwareManager
{
private:
    float getTemperature(Jetsonhardware);

public:
    Jetson(std::string name, std::vector<float> thresholds, Logger& _l,
           Config& c, Interface& i) :
      HardwareManager(name, thresholds, _l, c, i)
    {}
    int getAvailableMem() override;
    std::vector<float> getTemperatures() override;
};
