#pragma once

/*
 * This task is responsible for changing threshold of the detection via
 * network interface using HTTP
 * */
#include <atomic>

void runServer(float& threshVal, std::atomic<bool>& exit_flag);
