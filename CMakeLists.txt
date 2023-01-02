cmake_minimum_required(VERSION 3.13)
project(IOT)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(libs/services)

set(LIB_DIR "${CMAKE_SOURCE_DIR}/libs")
set(DARKNET_DEPS "${CMAKE_SOURCE_DIR}/darknet-deps")

if (DEFINED IMAGE_DBG)
  add_definitions("-DIMAGE_DBG")
endif (DEFINED IMAGE_DBG)

#add debug flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
#add flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#add specific flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOPENCV  -DCUDNN -DCUDNN_HALF")

#add flag for warning
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wfatal-errors -Wno-unused-result -Wno-unknown-pragmas")

#add math lib to iot
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lm -lcudnn -lstdc++ -lgomp -lcuda -pthread -lcublas")

#other flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Ofast  -fopenmp")

execute_process(
    COMMAND bash "-c" "pkg-config --cflags opencv4 2> /dev/null || pkg-config --cflags opencv"
    OUTPUT_VARIABLE opencv_cxxflags
)
execute_process(
    COMMAND bash "-c" "pkg-config --libs opencv4 2> /dev/null || pkg-config --libs opencv"
    OUTPUT_VARIABLE opencv_cxxflags2
)
string(STRIP "${opencv_cxxflags}"  opencv_cxxflags)
string(STRIP "${opencv_cxxflags2}" opencv_cxxflags2)
set (SOURCES "src/yolo_console_dll.cpp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${opencv_cxxflags} ${opencv_cxxflags2}")

link_directories(/usr/local/cuda/lib64 /usr/local/cudnn/lib64 
	${DARKNET_DEPS}/pre-built )

add_executable(iot ${SOURCES} )

include_directories(${CMAKE_SOURCE_DIR}/include 
	${CMAKE_SOURCE_DIR}/libs/ 
	${CMAKE_SOURCE_DIR}/libs/services/include )

add_library(libs src/DiscordNotifier.cpp src/Notifier.cpp )

target_include_directories(iot PUBLIC /usr/local/cuda/include 
	/usr/local/cudnn/include usr/local/cuda/include/cuda
	${LIB_DIR}
	)

target_link_libraries(iot LINK_PUBLIC libs services libdarknet.so cudart curand 
	opencv_highgui opencv_imgcodecs opencv_imgproc opencv_core 
	opencv_videoio opencv_video)

add_custom_command(TARGET iot PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
				   ${DARKNET_DEPS}/data/ $<TARGET_FILE_DIR:iot>/data)
add_custom_command(TARGET iot PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
				   ${DARKNET_DEPS}/cfg/ $<TARGET_FILE_DIR:iot>/cfg)
