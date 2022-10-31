#include <config_mgr.h>
ConfigMgr& ConfigMgr::getInstance()
{
    static ConfigMgr cfg("/etc/iot-config.yaml");
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

#ifdef UNITTEST
    assert(a.getTimeForcus() == 2);
    assert(a.getTimeSkippingDectection() == 20);
    assert(a.getMinQueueEntryLimit() == 15);
    assert(a.getNotifyAPI() == std::string("http://localhost:1234/updated"));
    assert(a.getDelay4Cap() == 20);
    assert(a.getHTTPPort() == 18080);
    assert(a.getNamesFile() == std::string("abc.coco"));
    assert(a.getCfgFile() == std::string("abc.coco"));
    assert(a.getWeightFile() == std::string("abc.coco"));
    assert(fabs(a.getThreshold() - 0.6) < 0.001);
#endif
}
