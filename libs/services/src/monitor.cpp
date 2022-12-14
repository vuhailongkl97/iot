#include "monitor.h"
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

void HardwareManager::prepareLog() {
    std::string ret;
    int mem = getAvailableMem();
    auto cur_temp = std::move(getTemperatures());
    ret = std::move(std::to_string(mem));
    ret += " MB ";

    if (cur_temp.size() != m_temp_thresholds.size()) {
        l.error("difference size between thresholds and temp_vec");
    } else {
        bool overheat = false;
        for (size_t vi = 0; vi < cur_temp.size(); vi++) {
            if (cur_temp[vi] >= m_temp_thresholds[vi]) { overheat = true; }
            ret += " ";
            ret += std::to_string(cur_temp[vi]);
        }
        if (overheat) {
            inf.notify(NOTIFY_TYPE::MSG, "temperature is over threshold");
            l.warn("temperature is over threshold");
        }
        l.info(ret.c_str());
    }
}
void HardwareManager::monitor() {
    while (1) {
        prepareLog();
        std::this_thread::sleep_for(std::chrono::minutes(2));
    }
}
auto Jetson::getAvailableMem() -> int {

    const char* cmd = "free -m | tail -n 2 | head -n 1 | awk '{ print $7 }'";
    FILE* p = popen(cmd, "r");

    int mem = 0;
    fscanf(p, "%d", &mem);
    pclose(p);
    return mem;
}

auto Jetson::getTemperature(Jetsonhardware hw) -> float {
    char path[200];
    snprintf(path, sizeof(path),
             "/sys/devices/virtual/thermal/thermal_zone%d/temp", hw);
    std::fstream tempF(path, std::fstream::in);
    char ret[10] = {0};
    if (!tempF.is_open()) {
        inf.notify(NOTIFY_TYPE::MSG, "falure to open the temperature file\n");
        l.warn("falure to open the temperature file\n");
        return 0;
    }
    tempF.read(ret, 10);

    return atoi(ret) / 1000.0;
}

auto Jetson::getTemperatures() -> std::vector<float> {
    std::vector<float> temps;
    temps.reserve(static_cast<int>(Jetsonhardware::jEND));
    for (auto i = 0; i < static_cast<int>(Jetsonhardware::jEND); i++) {
        temps.push_back(getTemperature(static_cast<Jetsonhardware>(i)));
    }
    return temps;
}
