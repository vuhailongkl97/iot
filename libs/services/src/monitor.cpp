#include <iostream>
#include <fstream>
#include <map>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <cstdio>
#include "logger.h"

enum class hardware : int
{
    A0 = 0,
    CPU,
    GPU,
    END
};

int getAvailableMem()
{

    const char* cmd = "free -m | tail -n 2 | head -n 1 | awk '{ print $7 }'";
    FILE* p = popen(cmd, "r");

    int mem = 0;
    fscanf(p, "%d", &mem);
    pclose(p);
    return mem;
}

static float getTemperature(hardware hw)
{

    char path[200];
    snprintf(path, sizeof(path), "/sys/devices/virtual/thermal/thermal_zone%d/temp", hw);
    std::fstream tempF(path, std::fstream::in);
    char ret[10] = {0};
    if (tempF.is_open() == false) {
        std::cout << "falure to open the temperature file\n";
        return 0;
    }
    tempF.read(ret, 10);

    return atoi(ret) / 1000.0;
}

static bool monitorTemperatureTask(std::shared_ptr<spdlog::logger> logger, std::vector<unsigned int> thresholds, unsigned int stamp)
{
    while (1) {

        std::string str;
        for (auto i = 0; i < (int)hardware::END; i++) {
            auto temp = getTemperature((hardware)i);
            if (temp > thresholds[(int)(i)]) {
                logger->critical(str);
                return false;
            }
            str += std::to_string(temp);
            str += " ";
        }
        str += std::to_string(getAvailableMem());
        str += "MB";
        logger->info("{}", str);
        std::this_thread::sleep_for(std::chrono::seconds(stamp));
    }
    return true;
}

void runMonitoring(std::atomic<bool>& exit_flag)
{

    std::vector<unsigned int> thresholds{60, 50, 50};
    auto logger = MLogger::getLoggerInstance();
    monitorTemperatureTask(logger, thresholds, 10);
    exit_flag = true;
}
