#include "crow.h"
#include "libs.h"
#include <atomic>
#include <cmath>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <mutex> // std::mutex, std::unique_lock
#include <queue>
#include <string>
#include <thread>
#include <vector>
// It makes sense only for video-Camera (not for video-File)
// To use - uncomment the following line. Optical-flow is supported only by
// OpenCV 3.x - 4.x
//#define TRACK_OPTFLOW
//#define GPU

// To use 3D-stereo camera ZED - uncomment the following line. ZED_SDK should be
// installed.
//#define ZED_STEREO

#include "yolo_v2_class.hpp" // imported functions from DLL

#ifdef OPENCV
#include <opencv2/core/version.hpp>
#include <opencv2/opencv.hpp> // C++
#ifndef CV_VERSION_EPOCH      // OpenCV 3.x and 4.x
#include <opencv2/videoio/videoio.hpp>
#define OPENCV_VERSION                                                         \
  CVAUX_STR(CV_VERSION_MAJOR)                                                  \
  "" CVAUX_STR(CV_VERSION_MINOR) "" CVAUX_STR(CV_VERSION_REVISION)
#ifndef USE_CMAKE_LIBS
#pragma comment(lib, "opencv_world" OPENCV_VERSION ".lib")
#ifdef TRACK_OPTFLOW
/*
#pragma comment(lib, "opencv_cudaoptflow" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_cudaimgproc" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_core" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_imgproc" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_highgui" OPENCV_VERSION ".lib")
*/
#endif // TRACK_OPTFLOW
#endif // USE_CMAKE_LIBS
#else  // OpenCV 2.x
#define OPENCV_VERSION                                                         \
  CVAUX_STR(CV_VERSION_EPOCH)                                                  \
  "" CVAUX_STR(CV_VERSION_MAJOR) "" CVAUX_STR(CV_VERSION_MINOR)
#ifndef USE_CMAKE_LIBS
#pragma comment(lib, "opencv_core" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_imgproc" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_highgui" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_video" OPENCV_VERSION ".lib")
#endif // USE_CMAKE_LIBS
#endif // CV_VERSION_EPOCH

#endif // OPENCV

template <typename T, typename U> struct DataResult {
  std::vector<T> s;
  std::vector<U> d;
  uint64_t frameid;
};

template <typename T> class DataUpdateListener {
public:
  using callback_function = void (*)(const T &data);
  void setCallback(callback_function cb) { callback_list.push_back(cb); }
  void onDataUpdate(const T &data) {
    for (auto cb : callback_list) {
      cb(data);
    }
  }

private:
  std::vector<callback_function> callback_list;
};

void mqtt_callback(const DataResult<int, int> &data) {
  std::cout << "mqtt_callback" << std::endl;
}

void console_callback(const DataResult<bbox_t, std::string> &data) {
  show_console_result(data.s, data.d, data.frameid);
}

template <typename T> class send_one_replaceable_object_t {
  const bool sync;
  std::atomic<T *> a_ptr;

public:
  void send(T const &_obj) {
    T *new_ptr = new T;
    *new_ptr = _obj;
    if (sync) {
      while (a_ptr.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }
    std::unique_ptr<T> old_ptr(a_ptr.exchange(new_ptr));
  }

  T receive() {
    std::unique_ptr<T> ptr;
    do {
      while (!a_ptr.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
      ptr.reset(a_ptr.exchange(NULL));
    } while (!ptr);
    T obj = *ptr;
    return obj;
  }

  bool is_object_present() { return (a_ptr.load() != NULL); }

  send_one_replaceable_object_t(bool _sync) : sync(_sync), a_ptr(NULL) {}
};

struct detection_data_t {
  cv::Mat cap_frame;
  std::shared_ptr<image_t> det_image;
  std::vector<bbox_t> result_vec;
  cv::Mat draw_frame;
  bool new_detection;
  uint64_t frame_id;
  bool exit_flag;
  cv::Mat zed_cloud;
  std::queue<cv::Mat> track_optflow_queue;
  detection_data_t() : new_detection(false), exit_flag(false) {}
};

void runServer(float &threshVal) {
#if 1
  crow::SimpleApp app;
  app.concurrency(1);
  CROW_ROUTE(app, "/threshold/<int>")
  ([&threshVal](int threshold) {
    if (threshold > 100 || threshold < 0)
      return crow::response(400);
    threshVal = threshold / 100.0;
    std::ostringstream os;
    os << "setting successfully";
    return crow::response(os.str());
  });
  app.port(18080).run();
#endif
}
int main(int argc, char *argv[]) {
  std::string names_file = "data/coco.names";
  std::string cfg_file = "cfg/yolov4-tiny.cfg";
  std::string weights_file = "data/yolov4-tiny.weights";
  std::string filename;
  DataUpdateListener<DataResult<bbox_t, std::string>> listener;

  // listener.setCallback(console_callback);
  if (argc > 4) { // voc.names yolo-voc.cfg yolo-voc.weights test.mp4
    names_file = argv[1];
    cfg_file = argv[2];
    weights_file = argv[3];
    filename = argv[4];
  } else if (argc > 1)
    filename = argv[1];

  float thresh = (argc > 5) ? std::stof(argv[5]) : 0.2;

  auto obj_names = objects_names_from_file(names_file);
  bool const send_network = false;     // true - for remote detection
  bool const use_kalman_filter = true; // true - for stationary camera

  bool detection_sync = true; // true - for video-file
  auto server = std::thread(runServer, std::ref(thresh));
  server.detach();

  while (true) {
    if (filename.size() == 0) {
      std::cout << "file name is empty, this program is exitting" << std::endl;
      break;
    }

    try {
      preview_boxes_t large_preview(100, 150, false),
          small_preview(50, 50, true);
      bool show_small_boxes = false;

      Detector detector(cfg_file, weights_file);
      std::string const file_ext =
          filename.substr(filename.find_last_of(".") + 1);
      std::string const protocol = filename.substr(0, 7);
      if (file_ext == "avi" || file_ext == "mp4" || file_ext == "mjpg" ||
          file_ext == "mov" || // video file
          protocol == "rtmp://" || protocol == "rtsp://" ||
          protocol == "http://" ||
          protocol == "https:/" || // video network stream
          filename == "zed_camera" || file_ext == "svo" ||
          filename == "web_camera") // ZED stereo camera

      {
        if (protocol == "rtsp://" || protocol == "http://" ||
            protocol == "https:/" || filename == "zed_camera" ||
            filename == "web_camera")
          detection_sync = false;

        cv::Mat cur_frame;
        std::atomic<int> fps_cap_counter(0), fps_det_counter(0);
        std::atomic<int> current_fps_cap(0), current_fps_det(0);
        std::atomic<bool> exit_flag(false);
        std::chrono::steady_clock::time_point steady_start, steady_end;
        int video_fps = 25;
        bool use_zed_camera = false;

        track_kalman_t track_kalman;

        cv::VideoCapture cap;
        if (filename == "web_camera") {
          cap.open(0);
          cap >> cur_frame;
        } else if (!use_zed_camera) {
          cap.open(filename);
          cap >> cur_frame;
        }
        video_fps = cap.get(cv::CAP_PROP_FPS);

        cv::Size const frame_size = cur_frame.size();
        std::cout << "\n Video size: " << frame_size << std::endl;

        const bool sync = detection_sync; // sync data exchange
        send_one_replaceable_object_t<detection_data_t> cap2prepare(sync),
            cap2draw(sync), prepare2detect(sync), detect2draw(sync),
            draw2show(sync), draw2write(sync), draw2net(sync);

        std::thread t_cap, t_prepare, t_detect, t_post, t_draw, t_write,
            t_network;

        // capture new video-frame
        if (t_cap.joinable())
          t_cap.join();
        t_cap = std::thread([&]() {
          uint64_t frame_id = 0;
          detection_data_t detection_data;
          do {
            detection_data = detection_data_t();
            cap >> detection_data.cap_frame;
            fps_cap_counter++;
            detection_data.frame_id = frame_id++;

            if (exit_flag) {
              detection_data.exit_flag = true;
            } else if (detection_data.cap_frame.empty()) {
              // verify and polling
              // here when cam is down
              std::cout << "retrying open " << filename << "\n";
              while (cap.open(filename) == false) {
                std::this_thread::sleep_for(std::chrono::milliseconds(3000));
              }
              video_fps = cap.get(cv::CAP_PROP_FPS);
              continue;
            }

            if (!detection_sync) {
              cap2draw.send(detection_data); // skip detection
            }
            cap2prepare.send(detection_data);
            std::this_thread::sleep_for(std::chrono::milliseconds(70));
          } while (!detection_data.exit_flag);
          std::cout << " t_cap exit \n";
        });

        // pre-processing video frame (resize,
        // convertion)
        t_prepare = std::thread([&]() {
          std::shared_ptr<image_t> det_image;
          detection_data_t detection_data;
          do {
            detection_data = cap2prepare.receive();

            det_image = detector.mat_to_image_resize(detection_data.cap_frame);
            detection_data.det_image = det_image;
            prepare2detect.send(detection_data); // detection

          } while (!detection_data.exit_flag);
          std::cout << " t_prepare exit \n";
        });

        // detection by Yolo
        if (t_detect.joinable())
          t_detect.join();
        t_detect = std::thread([&]() {
          std::shared_ptr<image_t> det_image;
          detection_data_t detection_data;
          do {
            detection_data = prepare2detect.receive();
            det_image = detection_data.det_image;
            std::vector<bbox_t> result_vec;

            if (det_image)
              result_vec = detector.detect_resized(*det_image, frame_size.width,
                                                   frame_size.height, thresh,
                                                   true); // true
            fps_det_counter++;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));

            detection_data.new_detection = true;
            detection_data.result_vec = result_vec;
            detect2draw.send(detection_data);
          } while (!detection_data.exit_flag);
          std::cout << " t_detect exit \n";
        });

        // draw rectangles (and track objects)
        t_draw = std::thread([&]() {
          std::queue<cv::Mat> track_optflow_queue;
          detection_data_t detection_data;
          do {
            // for Video-file
            if (detection_sync) {
              detection_data = detect2draw.receive();
            }
            // for Video-camera
            else {
              // get new Detection
              // result if present
              if (detect2draw.is_object_present()) {
                cv::Mat old_cap_frame =
                    detection_data.cap_frame; // use old captured frame
                detection_data = detect2draw.receive();
                if (!old_cap_frame.empty())
                  detection_data.cap_frame = old_cap_frame;
              }
              // get new Captured
              // frame
              else {
                std::vector<bbox_t> old_result_vec =
                    detection_data.result_vec; // use old detections
                detection_data = cap2draw.receive();
                detection_data.result_vec = old_result_vec;
              }
            }

            cv::Mat cap_frame = detection_data.cap_frame;
            cv::Mat draw_frame = detection_data.cap_frame.clone();
            std::vector<bbox_t> result_vec = detection_data.result_vec;

            // track ID by using kalman
            // filter
            if (use_kalman_filter) {
              if (detection_data.new_detection) {
                result_vec = track_kalman.correct(result_vec);
              } else {
                result_vec = track_kalman.predict();
              }
            }
            // track ID by using custom
            // function
            else {
              int frame_story = std::max(5, current_fps_cap.load());
              result_vec =
                  detector.tracking_id(result_vec, true, frame_story, 40);
            }

            if (use_zed_camera && !detection_data.zed_cloud.empty()) {
              result_vec =
                  get_3d_coordinates(result_vec, detection_data.zed_cloud);
            }

            // small_preview.set(draw_frame,
            // result_vec);
            // large_preview.set(draw_frame,
            // result_vec);
            draw_boxes(draw_frame, result_vec, obj_names, current_fps_det,
                       current_fps_cap, thresh);
            // show_console_result(result_vec,
            // obj_names,
            // detection_data.frame_id);
            DataResult<bbox_t, std::string> data_result{
                result_vec,
                obj_names,
                detection_data.frame_id,
            };
            listener.onDataUpdate(data_result);
            // large_preview.draw(draw_frame);
            // small_preview.draw(draw_frame,
            // true);

            detection_data.result_vec = result_vec;
            detection_data.draw_frame = draw_frame;
            // draw2show.send(detection_data);
            if (send_network)
              draw2net.send(detection_data);
          } while (!detection_data.exit_flag);
          std::cout << " t_draw exit \n";
        });

#if 0
				// show detection
				detection_data_t detection_data;
				do {
					steady_end =
					    std::chrono::steady_clock::now();
					float time_sec =
					    std::chrono::duration<double>(
						steady_end - steady_start)
						.count();
					if (time_sec >= 1) {
						current_fps_det =
						    fps_det_counter.load() /
						    time_sec;
						current_fps_cap =
						    fps_cap_counter.load() /
						    time_sec;
						steady_start = steady_end;
						fps_det_counter = 0;
						fps_cap_counter = 0;
					}

					detection_data = draw2show.receive();
					cv::Mat draw_frame =
					    detection_data.draw_frame;

					cv::imshow("window name", draw_frame);
					int key = cv::waitKey(3);  // 3 or 16ms
					if (key == 'f')
						show_small_boxes =
						    !show_small_boxes;
					if (key == 'p')
						while (true)
							if (cv::waitKey(100) ==
							    'p')
								break;
					// if (key == 'e') extrapolate_flag =
					// !extrapolate_flag;
					if (key == 27) {
						exit_flag = true;
					}

					// std::cout << " current_fps_det = " <<
					// current_fps_det << ", current_fps_cap
					// = " << current_fps_cap << std::endl;
				} while (!detection_data.exit_flag);
				std::cout << " show detection exit \n";

				cv::destroyWindow("window name");
#endif

        // wait for all threads
        if (t_cap.joinable())
          t_cap.join();
        if (t_prepare.joinable())
          t_prepare.join();
        if (t_detect.joinable())
          t_detect.join();
        if (t_post.joinable())
          t_post.join();
        if (t_draw.joinable())
          t_draw.join();

        // sleep here wait next event
        while (exit_flag) {
          std::cout << "sleep wait for next events\n";
          std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        break;
      }
    } catch (std::exception &e) {
      std::cerr << "exception: " << e.what() << "\n";
      // getchar();
    } catch (...) {
      std::cerr << "unknown exception \n";
      // getchar();
    }
  }

  return 0;
}
