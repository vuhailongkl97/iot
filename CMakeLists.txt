cmake_minimum_required(VERSION 3.0)
project(HELLO)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-DBACKWARD_HAS_UNWIND=1  -DBACKWARD_HAS_DW=1  -DBACKWARD_HAS_UNWIND=1)
add_subdirectory(backward-cpp)

# This will add backward.cpp to your target
#add_executable(mytarget main.cpp ${BACKWARD_ENABLE})

#add debug flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
#add flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#add specific flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOPENCV -DGPU  -DCUDNN -DCUDNN_HALF")

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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${opencv_cxxflags} ${opencv_cxxflags2}")

link_directories(/usr/local/cuda/lib64 /usr/local/cudnn/lib64 ./)
add_executable(iot yolo_console_dll.cpp ${BACKWARD_ENABLE} )

# This will add libraries, definitions and include directories needed by backward
# by setting each property on the target.
add_backward(iot)
add_library(libs lib.cpp)

target_include_directories(iot PUBLIC  /usr/local/cuda/include /usr/local/cudnn/include usr/local/cuda/include/cuda)

target_link_libraries(iot LINK_PUBLIC libs libdarknet.so cudart curand opencv_highgui opencv_imgcodecs opencv_imgproc opencv_core opencv_videoio opencv_video)

add_custom_command(TARGET iot PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:iot>/data)
add_custom_command(TARGET iot PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/cfg/ $<TARGET_FILE_DIR:iot>/cfg)
