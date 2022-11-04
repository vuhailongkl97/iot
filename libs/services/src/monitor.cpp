#include <iostream>
#include <fstream>
#include <map>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <cstdio>
#include "monitor.h"

int Jetson::getAvailableMem()
{

    const char* cmd = "free -m | tail -n 2 | head -n 1 | awk '{ print $7 }'";
    FILE* p = popen(cmd, "r");

    int mem = 0;
    fscanf(p, "%d", &mem);
    pclose(p);
    return mem;
}

float Jetson::getTemperature(Jetsonhardware hw)
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

std::vector<float> Jetson::getTemperatures()
{
    std::vector<float> temps;
    for (auto i = 0; i < (int)Jetsonhardware::END; i++) {
        temps.push_back(getTemperature((Jetsonhardware)i));
    }
    return temps;
}
