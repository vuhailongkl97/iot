#include <gtest/gtest.h>

#include "../src/logger.cpp"
#include "../src/config_mgr.cpp"
#include "../src/server.cpp"
#include "../src/monitor.cpp"

class fakeJetson : public HardwareManager
{
private:
    float getTemperature(Jetsonhardware);

public:
    fakeJetson(std::string name, std::vector<float> thresholds, Logger& _l,
               Config& c, Interface& i) :
      HardwareManager(name, thresholds, _l, c, i)
    {}
    int getAvailableMem() { return 0; }
    std::vector<float> getTemperatures() { return {1, 2, 3}; }
};

TEST(HARDWAREMONITOR, BASIC)
{

    Logger& lg = spdLogger().getInstance();
    Config& cfg = JSONConfig::getInstance("../iot-config.json");
    CrowServer p(cfg, lg);

    std::unique_ptr<HardwareManager> h(
      new fakeJetson("jetsonano", {1, 1, 1}, lg, cfg, p));

    h->monitor();
}
