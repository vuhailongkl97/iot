#pragma once
#include <atomic>
/*
 * Currently this monitor overheating Jetson nano board
 * when monitor task detect a problem it will set exit_flag to true
 * to stop main service ( detection )
 * */
void runMonitoring(std::atomic<bool> &exit_flag);
