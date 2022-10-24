#include <iostream>
#include <fstream>
#include <map>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

enum class hardware : int {
	A0 = 0,
	CPU,
	GPU,
    END	
};

static float getTemperature(hardware hw) {

    char path[200];
    snprintf(path, sizeof(path), "/sys/devices/virtual/thermal/thermal_zone%d/temp", hw);
	std::fstream tempF(path, std::fstream::in);
	char ret[10] = {0};
	if (tempF.is_open() == false)
	{
		std::cout << "falure to open the temperature file\n";
		return 0;
	}
	tempF.read(ret, 10);

	return atoi(ret)/1000.0;
}

static bool monitorTemperatureTask(std::shared_ptr<spdlog::logger> logger, std::vector<unsigned int> thresholds ,unsigned int stamp) {
	while(1) {
	
		std::string str;
		for(auto i = 0; i < (int)hardware::END; i++)
		{
			auto temp = getTemperature((hardware)i);
			if (temp > thresholds[(int)(i)]) {
				logger->critical(str);
				return false;
			}
			str += std::to_string(temp);
			str += " ";
		}
		logger->info("{}", str);
		std::this_thread::sleep_for(std::chrono::seconds(stamp));	
	}
	return true;
}

void runMonitoring(std::atomic<bool> &exit_flag) {

	std::vector<unsigned int> thresholds{60, 50, 50};
    auto max_size = 1048576 * 10;
    auto max_files = 3;
    auto logger = spdlog::rotating_logger_mt("some_logger_name", "logs/rotating.txt", max_size, max_files);

	spdlog::set_pattern("[%H:%M:%S] %v");
	spdlog::flush_every(std::chrono::seconds(10));
	monitorTemperatureTask(logger, thresholds, 10);
	exit_flag = true;
}
