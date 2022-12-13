# What it is

+ This is a customized example from [darknet](https://github.com/AlexeyAB/darknet/) for C++ in objects recognition in minimal configuration (without any dependency from the original repo ).

+ Run on embedded boards like Jetson Nano for create *a secured camera ( human detector )* 

# System Requirements 

+ [x] System detect human correctly
+ [x] User can Enable/Disable detection by *pose estimation* or commands
+ [x] System support configuring a part in an entire frame for detection (rectangle)
+ [x] Developer can change parameters( threshold, notifying APIs, hardware, ..)  via exported APIs
+ [x] System can monitor its hardware status (temperature, load) to reporting, prevent overload.
+ [x] Logging interval, when system met error it will report
+ [x] System export notifying about detection result
+ [x] System save changed configuration from User, still effect event if it is rebooted
+ [x] System don't spam notifying
+ [x] Software still work on other hardware which adapt requirements
+ [x] System will support register events when a new subscriber is created(dynamically IP) like buzzer, discord bot,...   
+ [ ] User can support detect multiple source at same time.
+ [ ] System support some commands like *take a picture*, *get current hardware status*

# Use Case Diagrams
![usecases](docs/usecases.drawio.png)

# Class diagram
Some main classes used :
* YamlConfig : Handle read config file in yaml format
* JsonConfig : Handle read config file in JSON format used in parsing json request from *interface* classes
* spdLogger: Logger Libraries wrapper
* JetsonNano: handle hardware request & monitoring
* Notifier: algorithms to filter the detected results and decided to notity or not
* CrowServer: handle/create HTTP requests from outside of this app (localhost)

![classes](docs/classDiagram.drawio.png)

# Activity diagram
Steps from getting a frame to decide notify or not :
![Activity](docs/activityDiagram.drawio.png)
# Dependencies
+ [opencv 4.1.1](https://pysource.com/2019/08/26/install-opencv-4-1-on-nvidia-jetson-nano/)
+ [cuda-10.2](https://jfrog.com/connect/post/installing-cuda-on-nvidia-jetson-nano/)
+ [asio](https://think-async.com/Asio/asio-1.24.0/doc/asio/using.html)
+ [curl](https://curl.se/download.html)

# How to run
Tested on Jetson nano 2GB.
## Build
```
git clone --recursive https://github.com/vuhailongkl97/iot.git
cd iot && git lfs pull
mkdir build && cd build
cmake -DIMAGE_DBG=1 ..  &&  make
--------------------
In development:
Run static analysis with clang-tidy. It required clang-tidy installed ( sudo apt install clang-tidy )
cmake  -DRUN_CLANG_TIDY=y .. && make
```
## Edit configuration and Run
### Edit configuration file
`/etc/iot-config.json` follow [config.json](https://github.com/vuhailongkl97/iot/blob/master/iot-config.json)
```
{
    "BoardName": "JetsonNano",
    "CfgFile": "yolov4-tiny.cfg", // < path to > iot/darknet-deps/cfg/yolov4-tiny.cfg
    "CorrectRate": 0.9,
    "Delay4CAP": 20, // if your board too overload by high Frames speed from source -> increase this to reduce captured Frames;
    "NameFile": "coco.names",
    "NotifyAPI": [ // after it have a result -> notify by create a HTTP request with **NotifyAPI** (also see below)
        "http://localhost:1234/updated"
    ],
    "Port": 18080, // Port for this app to change parameter dynamically
    "QUEUE_ENTRY_LIMIT_MIN": 15, // Use a queue whose size is 15 to save all Frames in TIME_FORCUS seconds
    "Src": "rtsp://admin:admin@192.168.1.3:554/cam/realmonitor?channel=4&subtype=1",
    "Status": false, // mean in working, is true mean disable
    "TIME_FORCUS": 2, // All Frames in 2sec are person will be consider to be a correct result
    "TIME_SKIP": 20, // After have detected result -> SKIP in next 20 secs
    "Threshold": 0.8999999761581421,
    "WeightFile": "yolov4-tiny.weights" // < path to > iot/darknet-deps/data/yolov4-tiny.weights
}
```

### Run
`./iot `  
a demo from my camera:  
![demo](docs/demo.png)

# Development
+ [x] 1. Testing stability
	+ - [x] Debugging via reporting results
	+ - [x] Aging 
	+ - [x] Correctness
- [x] 2. Develop notifying via HTTP requests

- [ ] support monitor multiple sources at a same time.

# Note: 
+ OS information `Linux jetson 4.9.140-tegra #1 SMP PREEMPT Fri Oct 16 12:32:46 PDT 2020 aarch64 aarch64 aarch64 GNU/Linux`
+ Install vino VNC follow `JetsonNano-RemoteVNCAccess.pdf`
+ Use vncviewer on your host in development 
	- Ubuntu ` sudo apt install xtightvncviewer -y`
+ Try to adjust your frame to fit with trained data set (416x416) in case of using yolov4-tiny.weights  
# APIs 
### Update configuration
Create Post request to the computer which is running detection with content like below
```
{
    "BoardName": "JetsonNano",
    "CfgFile": "yolov4-tiny.cfg",
    "CorrectRate": 0.9,
    "Delay4CAP": 20,
    "NameFile": "coco.names",
    "NotifyAPI": [
        "http://localhost:1234/updated"
    ],
    "Port": 18080,
    "QUEUE_ENTRY_LIMIT_MIN": 15,
    "Src": "rtsp://admin:admin@192.168.1.3:554/cam/realmonitor?channel=4&subtype=1",
    "Status": false,
    "TIME_FORCUS": 2,
    "TIME_SKIP": 20,
    "Threshold": 0.9,
    "WeightFile": "yolov4-tiny.weights"
}

ex:
1. save the configuration to a file *data.cfg*
2. /usr/bin/curl http://localhost:18080/config -X POST -d@data.cfg

```

### Notifying
When the program got a result frame it will create a post request to list of "NotifyAPI" as requested in *Update configuration* above

POST to `http://localhost:1234/updated"
contents in JSON format
```
{
    "content":"msg",
    "key":"a message may be a error report"
}
```
or

```
{
    "content":"img_path",
    "key":"<path to the detected frame saved in the img_path as a file>"
    // due to currently I use consumers on localhost itself therefor I just pass path to the image, when implement on other machine it should be updated.
}
```
# Libraries are used (as submodules)
+ [Crow](https://github.com/ipkn/crow) - socket management
+ [spdlog](https://github.com/gabime/spdlog) - logging 
+ [backward-cpp](https://github.com/bombela/backward-cpp) - a good backtrace when the system is crashed
+ [json](https://github.com/nlohmann/json) - read/write configuration in json

# Reference
+ [download_weights](https://github.com/AlexeyAB/darknet/blob/master/scripts/download_weights.ps1)

+ [dahua API](https://community.jeedom.com/uploads/short-url/tTQJPaNah7gZnU12VGGN9ZHEhOk.pdf)

# More
## Configure systemd
To make the app run as a service in your Linux system:  
follow [this](https://wiki.archlinux.org/title/systemd), and [my config](https://github.com/vuhailongkl97/iot/blob/master/iot.service)

## Docker config for jetson nano 
Inside docker [longkl/jetpack4_6_iot](https://hub.docker.com/r/longkl/jetpack4_6_iot) has:

Jetpack 32.7.1 
``` 
apt update
apt install cuda cmake pkg-config wget 
apt install build-essential

wget https://github.com/Kitware/CMake/releases/download/v3.25.1/cmake-3.25.1-linux-aarch64.sh
bash ./cmake-3.25.1-linux-aarch64.sh

https://sourceforge.net/projects/asio/files/asio/1.24.0%20%28Stable%29/
download unzip and make install

Lib Dependencies
https://repo.download.nvidia.com/jetson/#Jetpack%204.6.1
total 364M
-rw-rw-r-- 1 1000 1000 2.4K Nov 30 18:31 cuda-compiler-10-2_10.2.460-1_arm64.deb
-rw-rw-r-- 1 1000 1000 123K Nov 30 18:31 cuda-cudart-10-2_10.2.300-1_arm64.deb
-rw-rw-r-- 1 1000 1000 2.5K Nov 30 18:31 cuda-libraries-10-2_10.2.460-1_arm64.deb
-rw-rw-r-- 1 1000 1000  44M Nov 30 18:35 libcublas10_10.2.3.300-1_arm64.deb
-rw-rw-r-- 1 1000 1000 272M Nov 30 18:34 libcudnn8_8.2.1.32-1+cuda10.2_arm64.deb
-rw-rw-r-- 1 1000 1000  38M Nov 30 18:36 libcurand-10-2_10.1.2.300-1_arm64.deb
-rw-rw-r-- 1 1000 1000 999K Nov 30 18:36 libopencv-dev_4.1.1-2-gd5a58aa75_arm64.deb
-rw-rw-r-- 1 1000 1000 9.5M Nov 30 18:36 libopencv_4.1.1-2-gd5a58aa75_arm64.deb

```
