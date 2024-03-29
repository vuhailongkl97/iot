#include <config_mgr.h>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

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

    auto operator[](std::string k) -> json& { return m_config[k]; }

    auto begin() -> json::iterator { return m_config.begin(); }

    auto end() -> json::iterator { return m_config.end(); }

    auto contains(std::string k) const -> bool const { return m_config.contains(k); }
    friend auto operator<<(std::ostream& os,
                                    const JSONConfig::impl& i) -> std::ostream&;
};

auto operator<<(std::ostream& os, const JSONConfig::impl& j) -> std::ostream&
{
    os << std::setw(4) << j.m_config << std::endl;
    return os;
}

auto JSONConfig::status() -> bool
{
    if ((*m_config).contains("Status")) {
        return (*m_config)["Status"].get<bool>();
}
    throw std::runtime_error(__func__);
}

auto JSONConfig::parse(std::string cfg) -> bool
{
    using json = nlohmann::json;
    auto tmp_cfg = json::parse(cfg);
    for (const auto& i : tmp_cfg.items()) {
        if (!m_config->contains(i.key())) { continue; }
        auto& node = (*m_config)[i.key()];
        if (node.is_array() && !i.value().is_array()) {
            auto n = std::find(node.begin(), node.end(), i.value());
            if (n == node.end()) { node.push_back(i.value()); }
            continue;
        }
        node = i.value();
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

auto JSONConfig::getInstance(const char* cfg_path) -> Config&
{
    static JSONConfig cfg(cfg_path, new impl);
    return cfg;
}

auto JSONConfig::getTimeForcus() -> time_t
{
    if ((*m_config).contains("TIME_FORCUS")) {
        return (*m_config)["TIME_FORCUS"].get<time_t>();
}
    throw std::runtime_error(__func__);
}

auto JSONConfig::getTimeSkippingDetection() -> time_t
{
    if ((*m_config).contains("TIME_SKIP")) {
        return (*m_config)["TIME_SKIP"].get<time_t>();
}

    throw std::runtime_error(__func__);
}

auto JSONConfig::getMinQueueEntryLimit() -> int
{
    if ((*m_config).contains("QUEUE_ENTRY_LIMIT_MIN")) {
        return (*m_config)["QUEUE_ENTRY_LIMIT_MIN"].get<int>();
}
    throw std::runtime_error(__func__);
}

auto JSONConfig::getNotifyAPI() -> std::vector<std::string>
{
    if ((*m_config).contains("NotifyAPI")) {
        return (*m_config)["NotifyAPI"].get<std::vector<std::string>>();
}
    throw std::runtime_error(__func__);
}

auto JSONConfig::getDelay4Cap() -> int
{
    if ((*m_config).contains("Delay4CAP")) {
        return (*m_config)["Delay4CAP"].get<int>();
}

    throw std::runtime_error(__func__);
}

auto JSONConfig::getHTTPPort() -> int
{
    if ((*m_config).contains("Port")) { return (*m_config)["Port"].get<int>();
}
    throw std::runtime_error(__func__);
}

auto JSONConfig::getNamesFile() -> std::string
{
    if ((*m_config).contains("NameFile")) {
        return (*m_config)["NameFile"].get<std::string>();
}
    throw std::runtime_error(__func__);
}

auto JSONConfig::getCfgFile() -> std::string
{
    if ((*m_config).contains("CfgFile")) {
        return (*m_config)["CfgFile"].get<std::string>();
}
    throw std::runtime_error(__func__);
}

auto JSONConfig::getWeightFile() -> std::string
{
    if ((*m_config).contains("WeightFile")) {
        return (*m_config)["WeightFile"].get<std::string>();
}
    throw std::runtime_error(__func__);
}

auto JSONConfig::getThreshold() -> float
{
    if ((*m_config).contains("Threshold")) {
        return (*m_config)["Threshold"].get<float>();
}
    throw std::runtime_error(__func__);
}

void JSONConfig::setThreshold(float th) { (*m_config)["Threshold"] = th; }

auto JSONConfig::getSrc() -> std::string
{
    if ((*m_config).contains("Src")) {
        return (*m_config)["Src"].get<std::string>();
}

    throw std::runtime_error(__func__);
}

auto JSONConfig::getCorrectRate() -> float
{
    if ((*m_config).contains("CorrectRate")) {
        return (*m_config)["CorrectRate"].get<float>();
}
    throw std::runtime_error(__func__);
}

auto JSONConfig::getBoardName() -> std::string
{
    if ((*m_config).contains("BoardName")) {
        return (*m_config)["BoardName"].get<std::string>();
}
    return "Unknown";
}
void JSONConfig::show()
{
    for (const auto& i : (*m_config).m_config.items()) {
        std::cout << i.key() << ":" << i.value() << "\n";
    }
}
