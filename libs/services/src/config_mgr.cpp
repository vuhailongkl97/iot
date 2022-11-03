#include <config_mgr.h>

#ifndef DEFAULT_CFG_PATH 
#define DEFAULT_CFG_PATH "/etc/iot-config.yaml"
#endif

ConfigMgr& ConfigMgr::getInstance()
{
    static ConfigMgr cfg(DEFAULT_CFG_PATH);
    return cfg;
}

time_t ConfigMgr::getTimeForcus()
{
    if (m_config["TIME_FORCUS"]) return m_config["TIME_FORCUS"].as<time_t>();
    throw std::runtime_error(__func__);
}

time_t ConfigMgr::getTimeSkippingDectection()
{
    if (m_config["TIME_SKIP"]) return m_config["TIME_SKIP"].as<time_t>();
    throw std::runtime_error(__func__);
}

int ConfigMgr::getMinQueueEntryLimit()
{
    if (m_config["QUEUE_ENTRY_LIMIT_MIN"])
        return m_config["QUEUE_ENTRY_LIMIT_MIN"].as<int>();
    throw std::runtime_error(__func__);
}

std::string ConfigMgr::getNotifyAPI()
{
    if (m_config["NotifyAPI"]) return m_config["NotifyAPI"].as<std::string>();
    throw std::runtime_error(__func__);
}

int ConfigMgr::getDelay4Cap()
{
    if (m_config["Delay4CAP"]) return m_config["Delay4CAP"].as<int>();
    throw std::runtime_error(__func__);
}

int ConfigMgr::getHTTPPort()
{
    if (m_config["Port"]) return m_config["Port"].as<int>();
    throw std::runtime_error(__func__);
}

std::string ConfigMgr::getNamesFile()
{
    if (m_config["NameFile"]) return m_config["NameFile"].as<std::string>();
    throw std::runtime_error(__func__);
}

std::string ConfigMgr::getCfgFile()
{
    if (m_config["CfgFile"]) return m_config["CfgFile"].as<std::string>();
    throw std::runtime_error(__func__);
}

std::string ConfigMgr::getWeightFile()
{
    if (m_config["WeightFile"]) return m_config["WeightFile"].as<std::string>();
    throw std::runtime_error(__func__);
}

float ConfigMgr::getThreshold()
{
    if (m_config["Threshold"]) return m_config["Threshold"].as<float>();
    throw std::runtime_error(__func__);
}

void ConfigMgr::setThreshold(float th) { m_config["Threshold"] = th; }


std::string ConfigMgr::getSrc()
{
    if (m_config["Src"]) return m_config["Src"].as<std::string>();
    throw std::runtime_error(__func__);
}

float ConfigMgr::getCorrectRate()
{
    if (m_config["CorrectRate"]) return m_config["CorrectRate"].as<float>();
    throw std::runtime_error(__func__);
}

std::string ConfigMgr::getBoardName()
{
    if (m_config["BoardName"]) return m_config["BoardName"].as<std::string>();
    return "Unknown";
}
void ConfigMgr::show()
{
    for (auto i : m_config) {
        std::cout << i.first << ":" << i.second << "\n";
    }
}

void testConfig(ConfigMgr& cfg)
{
    cfg.show();
    cfg.getTimeForcus();
    cfg.getTimeSkippingDectection();
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
