#pragma once
#include <atomic>
#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <logger.h>
#include <config_mgr.h>
#include <server.h>

enum class Jetsonhardware : int
{
    A0 = 0,
    CPU,
    GPU,
    END
};

class HardwareManager
{
private:
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

    std::string prepareLog()
    {
        std::string ret;
        int mem = getAvailableMem();
        auto cur_temp = getTemperatures();
        ret = std::to_string(mem);
        ret += " MB ";

        if (cur_temp.size() != m_temp_thresholds.size()) {
            l.error("difference size between thresholds and temp_vec");
        }
        else {
            for (size_t vi = 0; vi < cur_temp.size(); vi++) {
                if (cur_temp[vi] >= m_temp_thresholds[vi]) {
                    inf.notify(NOTIFY_TYPE::MSG,
                               "temperature is over threshold");
                    l.warn("temperature is over threshold");
                }
                else {
                    ret += " ";
                    ret += std::to_string(cur_temp[vi]);
                }
            }
        }
        return ret;
    }
    void monitor()
    {
        while (1) {
            prepareLog();
            std::this_thread::sleep_for(std::chrono::minutes(2));
        }
    }
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
