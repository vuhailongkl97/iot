#include "yaml-cpp/yaml.h"
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <exception>
#include <cassert>
class ConfigMgr
{
private:
    const std::string m_file;
    YAML::Node m_config;
    explicit ConfigMgr(const std::string path) : m_file(path)
    {
        m_config = YAML::LoadFile(m_file);
    }

public:
    static ConfigMgr& getInstance();

    time_t getTimeForcus();
    time_t getTimeSkippingDectection();
    int getMinQueueEntryLimit();
    std::string getNotifyAPI();
    int getDelay4Cap();
    int getHTTPPort();
    std::string getNamesFile();
    std::string getCfgFile();
    std::string getWeightFile();
    float getThreshold();
    std::string getSrc();
    void show();
    YAML::Node operator[](std::string k) { return m_config[k]; }
    void sync()
    {
        std::ofstream fout("/etc/iot-config.yaml");
        fout << m_config;
    }
};

void testConfig(ConfigMgr&);
