cmake_minimum_required(VERSION 3.14)
project (services)
include(FetchContent)

set (SRC src/logger.cpp src/monitor.cpp src/server.cpp src/config_mgr.cpp)

include_directories(${CMAKE_SOURCE_DIR}/libs/services/include
	${CMAKE_SOURCE_DIR}/libs/dependencies/Crow/include 
	${CMAKE_SOURCE_DIR}/libs/dependencies/spdlog/include 
)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

add_library(services ${SRC})

target_link_libraries(
  services 
  nlohmann_json::nlohmann_json
)
