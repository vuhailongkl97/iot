#include "lib.h"
#include <sstream>
#include "services/include/monitor.h"
#include "services/include/server.h"
#include "services/include/config_mgr.h"
#include <atomic>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
//#define IMAGE_DBG

#define TRACK_OPTFLOW
//#define GPU

#include "yolo_v2_class.hpp"
#include <opencv2/core/version.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio/videoio.hpp>

template<typename T>
class send_one_replaceable_object_t
{
    const bool sync;
    std::atomic<T*> a_ptr;

public:
    void send(T const& _obj)
    {
        T* new_ptr = new T;
        *new_ptr = _obj;
        if (sync) {
            while (a_ptr.load())
                std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }
        std::unique_ptr<T> old_ptr(a_ptr.exchange(new_ptr));
    }

    T receive()
    {
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

struct detection_data_t
{
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

void draw_boxes(cv::Mat mat_img, std::vector<bbox_t> result_vec,
                std::vector<std::string> obj_names, int current_det_fps,
                int current_cap_fps, float thresh)
{
    for (auto& i : result_vec) {
        cv::Scalar color = obj_id_to_color(i.obj_id);
        if (obj_names.size() > i.obj_id) {
            std::string obj_name = obj_names[i.obj_id];
            if (obj_name != std::string("person")) { continue; }
            cv::rectangle(mat_img, cv::Rect(i.x, i.y, i.w, i.h), color, 1);
            if (i.track_id > 0) obj_name = std::to_string(i.track_id);
	    obj_name += " - " ;

            std::ostringstream oss;
            oss << std::setprecision(2) << i.prob;
            obj_name += oss.str();
            cv::Size const text_size =
            getTextSize(obj_name, cv::FONT_HERSHEY_COMPLEX_SMALL, 0.7, 1, 0);
            int max_width =
              (text_size.width > i.w + 2) ? text_size.width : (i.w + 2);
            max_width = std::max(max_width, (int)i.w + 2);
            cv::rectangle(
              mat_img,
              cv::Point2f(std::max((int)i.x - 1, 0),
                          std::max((int)i.y - 35, 0)),
              cv::Point2f(std::min((int)i.x + max_width, mat_img.cols - 1),
                          std::min((int)i.y, mat_img.rows - 1)),
              color, CV_FILLED, 8, 0);
            putText(mat_img, obj_name, cv::Point2f(i.x, i.y - 16),
                    cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cv::Scalar(0, 0, 0),
                    1);
        }
    }
    if (current_det_fps >= 0 && current_cap_fps >= 0) {
        std::string fps_str = "FPS det: " + std::to_string(current_det_fps) +
                              " FPS cap: " + std::to_string(current_cap_fps) +
                              " " + std::to_string(thresh);
        putText(mat_img, fps_str, cv::Point2f(10, 20),
                cv::FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(50, 255, 0), 1);
    }
}

std::vector<std::string> objects_names_from_file(std::string const filename)
{
    std::ifstream file(filename);
    std::vector<std::string> file_lines;
    if (!file.is_open()) return file_lines;
    for (std::string line; getline(file, line);)
        file_lines.push_back(line);
    std::cout << "object names loaded \n";
    return file_lines;
}

std::vector<obj_t> convertBbox2obj(const std::vector<bbox_t>& v)
{
    std::vector<obj_t> ret;
    for (auto& elem : v) {
        ret.push_back(
          {elem.prob, elem.obj_id, elem.track_id, elem.frames_counter});
    }
    return ret;
}

void customizedFrame(cv::Mat& f)
{
    // resize and crop to fit with 416x416 (trained data set)
    // current frame is 705x 578 -> 0x75 to 4aa x 4bb
    float scale = 0.75f;
    int scaled_width = f.size().width * scale;
    int scaled_height = f.size().height * scale;
    if (scaled_width > 400 && scaled_height > 400) {
        cv::resize(f, f, cv::Size(scaled_width, scaled_height),
                   cv::INTER_LINEAR);
        f = f(cv::Range(10, scaled_height), cv::Range(40, 460));
    }
}

int main(int argc, char* argv[])
{
    Logger& lg = spdLogger::getInstance();
    Config& cfg = JSONConfig::getInstance("/etc/iot-config.json");
    Interface& inf = CrowServer::getInstance(cfg, lg);
    Jetson jet("jetsonNano", {60, 50, 50}, lg, cfg, inf);
    HardwareManager& hw = jet;
    testConfig(cfg);

    std::string names_file = cfg.getNamesFile();
    std::string cfg_file = cfg.getCfgFile();
    std::string weights_file = cfg.getWeightFile();
    std::string filename = cfg.getSrc();
    DataUpdateListener listener;

    listener.addSubscriber(new discordNotifier(cfg, lg, inf));

    auto obj_names = objects_names_from_file(names_file);
    bool const send_network = false;     // true - for remote detection
    bool const use_kalman_filter = true; // true - for stationary camera
    Detector detector(cfg_file, weights_file);

    bool detection_sync = true; // true - for video-file

#ifdef TRACK_OPTFLOW // for slow GPU
    detection_sync = false;
    Tracker_optflow tracker_flow;
#endif // TRACK_OPTFLOW

    std::atomic<bool> exit_flag(false);

    if (cfg.getBoardName() == std::string("JetsonNano")) {
        auto thermometer = std::thread([&] { hw.monitor(); });
        thermometer.detach();
    }

    inf.initialize();
    auto server = std::thread([&] { inf.run(); });
    server.detach();

    while (true) {
        if (filename.size() == 0) {
            std::cout << "file name is empty, this program is exitting"
                      << std::endl;
            break;
        }

        try {
            std::string const file_ext =
              filename.substr(filename.find_last_of(".") + 1);
            std::string const protocol = filename.substr(0, 7);
            if (file_ext == "avi" || file_ext == "mp4" || file_ext == "mjpg" ||
                file_ext == "mov" || protocol == "rtmp://" || protocol == "rtsp://" ||
                protocol == "http://" || protocol == "https:/")
            {
                if (protocol == "rtsp://" || protocol == "http://" || protocol == "https:/" )
                    detection_sync = false;

                cv::Mat cur_frame;
                std::atomic<int> fps_cap_counter(0), fps_det_counter(0);
                std::atomic<int> current_fps_cap(0), current_fps_det(0);
                std::chrono::steady_clock::time_point steady_start, steady_end;
                track_kalman_t track_kalman;
                cv::VideoCapture cap;

				cap.open(cfg.getSrc());
				cap >> cur_frame;

                customizedFrame(cur_frame);
                cv::Size const frame_size = cur_frame.size();
                std::cout << "\n Video size: " << frame_size << std::endl;

                const bool sync = detection_sync; // sync data exchange
                send_one_replaceable_object_t<detection_data_t> cap2prepare(
                  sync),
                  cap2draw(sync), prepare2detect(sync), detect2draw(sync),
                  draw2show(sync), draw2write(sync), draw2net(sync);

                std::thread t_cap, t_prepare, t_detect, t_post, t_draw, t_write,
                  t_network;

                // capture new video-frame
                if (t_cap.joinable()) t_cap.join();
                t_cap = std::thread([&]() {
                    uint64_t frame_id = 0;
                    detection_data_t detection_data;
                    do {
                        detection_data = detection_data_t();
                        cap >> detection_data.cap_frame;
                        customizedFrame(detection_data.cap_frame);
                        fps_cap_counter++;
                        detection_data.frame_id = frame_id++;

                        exit_flag = cfg.status();
                        if (exit_flag) { detection_data.exit_flag = true; }
                        else if (detection_data.cap_frame.empty()) {
                            std::cout << "retrying open " << filename << "\n";
                            while (cap.open(filename) == false) {
                                std::this_thread::sleep_for(
                                  std::chrono::milliseconds(3000));
                            }
                            continue;
                        }

                        if (!detection_sync) {
                            cap2draw.send(detection_data); // skip detection
                        }
                        cap2prepare.send(detection_data);
                        std::this_thread::sleep_for(
                          std::chrono::milliseconds(cfg.getDelay4Cap()));
                    } while (!detection_data.exit_flag);
                    std::cout << " t_cap exit \n";
                });

                // pre-processing video frame (resize, convertion)
                t_prepare = std::thread([&]() {
                    std::shared_ptr<image_t> det_image;
                    detection_data_t detection_data;
                    do {
                        detection_data = cap2prepare.receive();

                        det_image = detector.mat_to_image_resize(
                          detection_data.cap_frame);
                        detection_data.det_image = det_image;
                        prepare2detect.send(detection_data); // detection

                    } while (!detection_data.exit_flag);
                    std::cout << " t_prepare exit \n";
                });

                // detection by Yolo
                t_detect = std::thread([&]() {
                    std::shared_ptr<image_t> det_image;
                    detection_data_t detection_data;
                    do {
                        detection_data = prepare2detect.receive();
                        det_image = detection_data.det_image;
                        std::vector<bbox_t> result_vec;

                        if (det_image)
                            result_vec = detector.detect_resized(
                              *det_image, frame_size.width, frame_size.height,
                              cfg.getThreshold(),
                              true); // true
                        fps_det_counter++;
                        std::this_thread::sleep_for(
                          std::chrono::milliseconds(20));

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
                            if (detect2draw.is_object_present()) {
                                cv::Mat old_cap_frame = detection_data.cap_frame;
                                detection_data = detect2draw.receive();
                                if (!old_cap_frame.empty())
                                    detection_data.cap_frame = old_cap_frame;
                            }
                            // get new Captured frame
                            else {
                                std::vector<bbox_t> old_result_vec =
                                  detection_data.result_vec; // use old detections
                                detection_data = cap2draw.receive();
                                detection_data.result_vec = old_result_vec;
                            }
                        }

                        cv::Mat cap_frame = detection_data.cap_frame;
                        cv::Mat draw_frame = detection_data.cap_frame.clone();
                        std::vector<bbox_t> result_vec =
                          detection_data.result_vec;
#ifdef TRACK_OPTFLOW
                        if (detection_data.new_detection) {
                            tracker_flow.update_tracking_flow(
                              detection_data.cap_frame,
                              detection_data.result_vec);
                            while (track_optflow_queue.size() > 0) {
                                draw_frame = track_optflow_queue.back();
                                result_vec = tracker_flow.tracking_flow(
                                  track_optflow_queue.front(), false);
                                track_optflow_queue.pop();
                            }
                        } else {
                            track_optflow_queue.push(cap_frame);
                            result_vec =
                              tracker_flow.tracking_flow(cap_frame, false);
                        }
                        detection_data.new_detection =
                          true; // to correct kalman filter
#endif                          //TRACK_OPTFLOW

                        // track ID by using kalman
                        // filter
                        if (use_kalman_filter) {
                            if (detection_data.new_detection) {
                                result_vec = track_kalman.correct(result_vec);
                            }
                            else {
                                result_vec = track_kalman.predict();
                            }
                        }
                        // track ID by using custom
                        // function
                        else {
                            int frame_story =
                              std::max(5, current_fps_cap.load());
                            result_vec = detector.tracking_id(result_vec, true,
                                                              frame_story, 40);
                        }

                        draw_boxes(draw_frame, result_vec, obj_names,
                                   current_fps_det, current_fps_cap,
                                   cfg.getThreshold());

                        DataResult data_result{draw_frame,
                                               convertBbox2obj(result_vec),
                                               obj_names, current_fps_det};
                        listener.onDataUpdate(data_result);

                        detection_data.result_vec = result_vec;
                        detection_data.draw_frame = draw_frame;
                        steady_end = std::chrono::steady_clock::now();
                        float time_sec = std::chrono::duration<double>(
                                           steady_end - steady_start)
                                           .count();
                        if (time_sec >= 1) {
                            current_fps_det = fps_det_counter.load() / time_sec;
                            current_fps_cap = fps_cap_counter.load() / time_sec;
                            steady_start = steady_end;
                            fps_det_counter = 0;
                            fps_cap_counter = 0;
                        }

#ifdef IMAGE_DBG
                        draw2show.send(detection_data);
#endif
                        if (send_network) draw2net.send(detection_data);
                    } while (!detection_data.exit_flag);
                    std::cout << " t_draw exit \n";
                });

#ifdef IMAGE_DBG
                // show detection
                detection_data_t detection_data;
                do {
                    steady_end = std::chrono::steady_clock::now();
                    float time_sec =
                      std::chrono::duration<double>(steady_end - steady_start)
                        .count();
                    if (time_sec >= 1) {
                        current_fps_det = fps_det_counter.load() / time_sec;
                        current_fps_cap = fps_cap_counter.load() / time_sec;
                        steady_start = steady_end;
                        fps_det_counter = 0;
                        fps_cap_counter = 0;
                    }

                    detection_data = draw2show.receive();
                    cv::Mat draw_frame = detection_data.draw_frame;

                    cv::imshow("window name", draw_frame);
                    int key = cv::waitKey(3); // 3 or 16ms
                    if (key == 'p')
                        while (true)
                            if (cv::waitKey(100) == 'p') break;
                    if (key == 27) { exit_flag = true; }

                } while (!detection_data.exit_flag);
                std::cout << " show detection exit \n";

                cv::destroyWindow("window name");
#endif
                // wait for all threads
                if (t_cap.joinable()) t_cap.join();
                if (t_prepare.joinable()) t_prepare.join();
                if (t_detect.joinable()) t_detect.join();
                if (t_draw.joinable()) t_draw.join();

                // sleep here wait next event
                while (exit_flag) {
                    exit_flag = cfg.status();
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                }
            }
        }
        catch (std::exception& e) {
            std::cerr << "exception: " << e.what() << "\n";
        }
        catch (...) {
            std::cerr << "unknown exception \n";
        }
    }
    return 0;
}
