#include <config_mgr.h>
#include "yaml-cpp/yaml.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <iomanip>

struct YamlConfig::impl
{
    YAML::Node m_config;

    YAML::Node operator[](std::string k) { return m_config[k]; }

    YAML::const_iterator begin() { return m_config.begin(); }

    YAML::const_iterator end() { return m_config.end(); }
    friend std::ostream& operator<<(std::ostream& os,
                                    const YamlConfig::impl& i);
};

std::ostream& operator<<(std::ostream& os, const YamlConfig::impl& i)
{
    os << i.m_config;
    return os;
}

void YamlConfig::sync()
{
    std::ofstream fout(m_file);
    fout << m_config->m_config;
}

YamlConfig::YamlConfig(const std::string path, impl* p) :
  m_file(path), m_config(p), Config()
{
    m_config->m_config = YAML::LoadFile(m_file);
}

Config& YamlConfig::getInstance(const char* cfg_path)
{
    static YamlConfig cfg(cfg_path, new impl);
    return cfg;
}

bool YamlConfig::status() {
    if ((*m_config)["Status"])
        return (*m_config)["Status"].as<bool>();
    throw std::runtime_error(__func__);
}
bool YamlConfig::parse(std::string cfg)
{
    m_config->m_config = YAML::Load(cfg);
    return true;
}

time_t YamlConfig::getTimeForcus()
{
    if ((*m_config)["TIME_FORCUS"])
        return (*m_config)["TIME_FORCUS"].as<time_t>();
    throw std::runtime_error(__func__);
}

time_t YamlConfig::getTimeSkippingDetection()
{
    if ((*m_config)["TIME_SKIP"]) return (*m_config)["TIME_SKIP"].as<time_t>();
    throw std::runtime_error(__func__);
}

int YamlConfig::getMinQueueEntryLimit()
{
    if ((*m_config)["QUEUE_ENTRY_LIMIT_MIN"])
        return (*m_config)["QUEUE_ENTRY_LIMIT_MIN"].as<int>();
    throw std::runtime_error(__func__);
}

std::string YamlConfig::getNotifyAPI()
{
    if ((*m_config)["NotifyAPI"])
        return (*m_config)["NotifyAPI"].as<std::string>();
    throw std::runtime_error(__func__);
}

int YamlConfig::getDelay4Cap()
{
    if ((*m_config)["Delay4CAP"]) return (*m_config)["Delay4CAP"].as<int>();
    throw std::runtime_error(__func__);
}

int YamlConfig::getHTTPPort()
{
    if ((*m_config)["Port"]) return (*m_config)["Port"].as<int>();
    throw std::runtime_error(__func__);
}

std::string YamlConfig::getNamesFile()
{
    if ((*m_config)["NameFile"])
        return (*m_config)["NameFile"].as<std::string>();
    throw std::runtime_error(__func__);
}

std::string YamlConfig::getCfgFile()
{
    if ((*m_config)["CfgFile"]) return (*m_config)["CfgFile"].as<std::string>();
    throw std::runtime_error(__func__);
}

std::string YamlConfig::getWeightFile()
{
    if ((*m_config)["WeightFile"])
        return (*m_config)["WeightFile"].as<std::string>();
    throw std::runtime_error(__func__);
}

float YamlConfig::getThreshold()
{
    if ((*m_config)["Threshold"]) return (*m_config)["Threshold"].as<float>();
    throw std::runtime_error(__func__);
}

void YamlConfig::setThreshold(float th) { (*m_config)["Threshold"] = th; }


std::string YamlConfig::getSrc()
{
    if ((*m_config)["Src"]) return (*m_config)["Src"].as<std::string>();
    throw std::runtime_error(__func__);
}

float YamlConfig::getCorrectRate()
{
    if ((*m_config)["CorrectRate"])
        return (*m_config)["CorrectRate"].as<float>();
    throw std::runtime_error(__func__);
}

std::string YamlConfig::getBoardName()
{
    if ((*m_config)["BoardName"])
        return (*m_config)["BoardName"].as<std::string>();
    return "Unknown";
}
void YamlConfig::show()
{
    for (auto i : (*m_config)) {
        std::cout << i.first << ":" << i.second << "\n";
    }
}

void testConfig(Config& cfg)
{
    cfg.show();
	cfg.status();
    cfg.getTimeForcus();
    cfg.getTimeSkippingDetection();
    cfg.getMinQueueEntryLimit();
    cfg.getNotifyAPI();
    cfg.getDelay4Cap();
    cfg.getHTTPPort();
    cfg.getNamesFile();
    cfg.getCfgFile();
    cfg.getWeightFile();
    cfg.getThreshold();
    cfg.getCorrectRate();
}

struct JSONConfig::impl
{
    using json = nlohmann::json;
    json m_config;

    json& operator[](std::string k) { return m_config[k]; }

    json::iterator begin() { return m_config.begin(); }

    json::iterator end() { return m_config.end(); }

    bool contains(std::string k) { return m_config.contains(k); }
    friend std::ostream& operator<<(std::ostream& os,
                                    const JSONConfig::impl& i);
};

std::ostream& operator<<(std::ostream& os, const JSONConfig::impl& j)
{
    os << std::setw(4) << j.m_config << std::endl;
    return os;
}

bool JSONConfig::status() {
    if ((*m_config).contains("Status"))
        return (*m_config)["Status"].get<bool>();
    throw std::runtime_error(__func__);
}

bool JSONConfig::parse(std::string cfg)
{
    using json = nlohmann::json;
    m_config->m_config = json::parse(cfg);
    return true;
}

void JSONConfig::sync()
{
    std::ofstream fout(m_file);
    fout << std::setw(4) << m_config->m_config;
}

JSONConfig::JSONConfig(const std::string path, impl* p) :
  m_file(path), m_config(p), Config()
{
    using json = nlohmann::json;
    std::ifstream f(m_file);
    m_config->m_config = json::parse(f);
}

Config& JSONConfig::getInstance(const char* cfg_path)
{
    static JSONConfig cfg(cfg_path, new impl);
    return cfg;
}

time_t JSONConfig::getTimeForcus()
{
    if ((*m_config).contains("TIME_FORCUS"))
        return (*m_config)["TIME_FORCUS"].get<time_t>();
    throw std::runtime_error(__func__);
}

time_t JSONConfig::getTimeSkippingDetection()
{
    if ((*m_config).contains("TIME_SKIP"))
        return (*m_config)["TIME_SKIP"].get<time_t>();

    throw std::runtime_error(__func__);
}

int JSONConfig::getMinQueueEntryLimit()
{
    if ((*m_config).contains("QUEUE_ENTRY_LIMIT_MIN"))
        return (*m_config)["QUEUE_ENTRY_LIMIT_MIN"].get<int>();
    throw std::runtime_error(__func__);
}

std::string JSONConfig::getNotifyAPI()
{
    if ((*m_config).contains("NotifyAPI"))
        return (*m_config)["NotifyAPI"].get<std::string>();
    throw std::runtime_error(__func__);
}

int JSONConfig::getDelay4Cap()
{
    if ((*m_config).contains("Delay4CAP"))
        return (*m_config)["Delay4CAP"].get<int>();

    throw std::runtime_error(__func__);
}

int JSONConfig::getHTTPPort()
{
    if ((*m_config).contains("Port")) return (*m_config)["Port"].get<int>();
    throw std::runtime_error(__func__);
}

std::string JSONConfig::getNamesFile()
{
    if ((*m_config).contains("NameFile"))
        return (*m_config)["NameFile"].get<std::string>();
    throw std::runtime_error(__func__);
}

std::string JSONConfig::getCfgFile()
{
    if ((*m_config).contains("CfgFile"))
        return (*m_config)["CfgFile"].get<std::string>();
    throw std::runtime_error(__func__);
}

std::string JSONConfig::getWeightFile()
{
    if ((*m_config).contains("WeightFile"))
        return (*m_config)["WeightFile"].get<std::string>();
    throw std::runtime_error(__func__);
}

float JSONConfig::getThreshold()
{
    if ((*m_config).contains("Threshold"))
        return (*m_config)["Threshold"].get<float>();
    throw std::runtime_error(__func__);
}

void JSONConfig::setThreshold(float th) { (*m_config)["Threshold"] = th; }

std::string JSONConfig::getSrc()
{
    if ((*m_config).contains("Src"))
        return (*m_config)["Src"].get<std::string>();

    throw std::runtime_error(__func__);
}

float JSONConfig::getCorrectRate()
{
    if ((*m_config).contains("CorrectRate"))
        return (*m_config)["CorrectRate"].get<float>();
    throw std::runtime_error(__func__);
}

std::string JSONConfig::getBoardName()
{
    if ((*m_config).contains("BoardName"))
        return (*m_config)["BoardName"].get<std::string>();
    return "Unknown";
}
void JSONConfig::show()
{
    for (auto i : (*m_config).m_config.items()) {
        std::cout << i.key() << ":" << i.value() << "\n";
    }
}
