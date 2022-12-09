#include "monitor.h"
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <vector>

auto Jetson::getAvailableMem() -> int
{

    const char* cmd = "free -m | tail -n 2 | head -n 1 | awk '{ print $7 }'";
    FILE* p = popen(cmd, "r");

    int mem = 0;
    fscanf(p, "%d", &mem);
    pclose(p);
    return mem;
}

auto Jetson::getTemperature(Jetsonhardware hw) -> float
{
    char path[200];
    snprintf(path, sizeof(path),
             "/sys/devices/virtual/thermal/thermal_zone%d/temp", hw);
    std::fstream tempF(path, std::fstream::in);
    char ret[10] = {0};
    if (!tempF.is_open()) {
        std::cout << "falure to open the temperature file\n";
        return 0;
    }
    tempF.read(ret, 10);

    return atoi(ret) / 1000.0;
}

auto Jetson::getTemperatures() -> std::vector<float>
{
    std::vector<float> temps;
    temps.reserve(static_cast<int>(Jetsonhardware::jEND));
	for (auto i = 0; i < static_cast<int>(Jetsonhardware::jEND); i++) {
        temps.push_back(getTemperature(static_cast<Jetsonhardware>(i)));
    }
    return temps;
}
