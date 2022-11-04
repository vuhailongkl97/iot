#include <iostream>
#include "services/include/monitor.h"
#include "services/include/server.h"
#include "services/include/config_mgr.h"
class fakeJetson : public HardwareManager
{
private:
    float getTemperature(Jetsonhardware) {return 0;}

public:
    fakeJetson(std::string name, std::vector<float> thresholds, Logger& _l,
               Config& c, Interface& i) :
      HardwareManager(name, thresholds, _l, c, i)
    {}
    int getAvailableMem() { return 0; }
    std::vector<float> getTemperatures() { return {1, 2, 3}; }
};

int main() {
    Logger& lg = spdLogger().getInstance();
    Config& cfg = JSONConfig::getInstance("../iot-config.json");
	Interface* f = new CrowServer (cfg, lg);
 //   Interface &server = ser;

	lg.info("hello");

   f->initialize();
  //  server.notify(NOTIFY_TYPE::DETECT_RET, "abcdef");


    fakeJetson jet("jetsonano", {1, 1, 1}, lg, cfg, *f);

	std::thread moni([&] {
   		jet.monitor();
					});
	std::thread t1([&]{
			f->run();
			});
	t1.join();
}
