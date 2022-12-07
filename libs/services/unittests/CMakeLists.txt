cmake_minimum_required(VERSION 3.14)
project(my_project)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

add_executable(
  cfg_mgr_test
  config_mgr_test.cc 
)

target_link_libraries(
  cfg_mgr_test
  GTest::gtest_main
  nlohmann_json::nlohmann_json
)

add_executable(
  logger_test 
  logger_test.cc 
)

target_link_libraries(
  logger_test
  GTest::gtest_main
)

add_executable(
  interface_test 
  interface_test.cc 
  ../src/logger.cpp
  ../src/config_mgr.cpp
)

target_link_libraries(
  interface_test
  GTest::gtest_main
  nlohmann_json::nlohmann_json
)

add_executable(
  hardware_test 
  hardware_test.cc 
)

target_link_libraries(
  hardware_test
  GTest::gtest_main
  nlohmann_json::nlohmann_json
)

include(GoogleTest)

include_directories(${CMAKE_SOURCE_DIR}/../../services/include
	${CMAKE_SOURCE_DIR}/../../dependencies/Crow/include 
	${CMAKE_SOURCE_DIR}/../../dependencies/spdlog/include 
)

target_link_libraries(
		cfg_mgr_test 
)
gtest_discover_tests(cfg_mgr_test)
gtest_discover_tests(hardware_test)
gtest_discover_tests(interface_test)
gtest_discover_tests(logger_test)
