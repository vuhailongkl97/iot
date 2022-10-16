# What it is
+ This is customized example from [darknet](https://github.com/AlexeyAB/darknet/) for C++ for recongize objects in minimal configuration.

+ Main purpose of this project is customize the darknet project to minimal version to can run on Jetson Nano for create *a security camera ( human detector )* 

+ This will support backend system for notifying

# Dependencies
+ [opencv 4.1.1](https://pysource.com/2019/08/26/install-opencv-4-1-on-nvidia-jetson-nano/)
+ [cuda-10.2](https://jfrog.com/connect/post/installing-cuda-on-nvidia-jetson-nano/)
# How to run
Tested on Jetson nano 2GB.
1. Clone this repository, and use `git lfs pull` to get the weight files
2. Do `mkdir build && cd build && cmake .. && make`
3. Output binary's name is *iot*, let run it follow parameter is video file or rtsp, http, https, rtmp.  
` ./iot rtsp://<path to source images `   
` ./iot  'rtsp://username:password@192.168.1.111:554/cam/realmonitor?channel=3&subtype=1'`  
ad-Mr123

# Development
1. Testing stability
	+ Debugging via reporting results
	+ Aging 
	+ Correctness
2. Develop backend system management with Golang

# Note: 
+ OS information `Linux jetson 4.9.140-tegra #1 SMP PREEMPT Fri Oct 16 12:32:46 PDT 2020 aarch64 aarch64 aarch64 GNU/Linux`
+ Install vino VNC follow `JetsonNano-RemoteVNCAccess.pdf`
+ Use vncviewer on your host in development 
	- Ubuntu ` sudo apt install xtightvncviewer -y`
+ Due to I use *yolov3.weight* as default but its size too big need to use [git large file](https://git-lfs.github.com/). 


# Reference
+ [download_weights](https://github.com/AlexeyAB/darknet/blob/master/scripts/download_weights.ps1)

+ [yolov3.weight](https://pjreddie.com/media/files/yolov3.weights)
+ [dahua API](https://community.jeedom.com/uploads/short-url/tTQJPaNah7gZnU12VGGN9ZHEhOk.pdf)
