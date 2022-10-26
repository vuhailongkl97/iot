cmake_minimum_required(VERSION 3.1)
project (services)

set (SRC src/logger.cpp src/monitor.cpp src/server.cpp)

include_directories(${CMAKE_SOURCE_DIR}/libs/services/include
	${CMAKE_SOURCE_DIR}/libs/dependencies/Crow/include 
	${CMAKE_SOURCE_DIR}/libs/dependencies/spdlog/include )

add_library(services ${SRC})

