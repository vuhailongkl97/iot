#include <config_mgr.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <iomanip>

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

bool JSONConfig::status()
{
    if ((*m_config).contains("Status"))
        return (*m_config)["Status"].get<bool>();
    throw std::runtime_error(__func__);
}

bool JSONConfig::parse(std::string cfg)
{
    using json = nlohmann::json;
    auto tmp_cfg = json::parse(cfg);
    for (auto i : tmp_cfg.items()) {
        if (m_config->contains(i.key())) {
            auto& node = (*m_config)[i.key()];
            bool existed = false;
            if (node.is_array() && !i.value().is_array()) {
                for (auto& v : node) {
                    if (v == i.value()) existed = true;
                }
                if (!existed) { node.push_back(i.value()); }
            }
            else {
                node = i.value();
            }
        }
        std::cout << i.key() << ":" << i.value() << "\n";
    }
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

std::vector<std::string> JSONConfig::getNotifyAPI()
{
    if ((*m_config).contains("NotifyAPI"))
        return (*m_config)["NotifyAPI"].get<std::vector<std::string>>();
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
