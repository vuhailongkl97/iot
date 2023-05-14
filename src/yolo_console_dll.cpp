#include <sstream>
#include <atomic>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <vector>
#include <memory>
#include <future>

#include <DiscordNotifier.hpp>
#include "services/include/monitor.h"
#include "services/include/server.h"
#include "services/include/config_mgr.h"

//#define IMAGE_DBG

#define TRACK_OPTFLOW
#include "main.hpp"
#include "customized.hpp"

void monitor()
{
    Logger& lg = spdLogger::getInstance();
    Config& cfg = JSONConfig::getInstance("/etc/iot-config.json");
    Interface& inf = CrowServer::getInstance(cfg, lg);
    Jetson jet(std::string("jetsonNano"), std::vector<float>{60, 50, 50}, lg, cfg, inf);
    HardwareManager& hw = jet;

    if (cfg.getBoardName() == std::string("JetsonNano")) {
        auto thermometer = std::thread([&] { hw.monitor(); });
        thermometer.detach();
		std::cout << "monitor is started\n";
    }
}

void serving() {
    Logger& lg = spdLogger::getInstance();
    Config& cfg = JSONConfig::getInstance("/etc/iot-config.json");
    Interface& inf = CrowServer::getInstance(cfg, lg);
    inf.initialize();

    auto server = std::thread([&] { inf.run(); });
    server.detach();
	std::cout << "serving is started\n";
}
int main(int argc, char* argv[]) {
    Logger& lg = spdLogger::getInstance();
    Config& cfg = JSONConfig::getInstance("/etc/iot-config.json");
    Interface& inf = CrowServer::getInstance(cfg, lg);

    testConfig(cfg);
#if 1

	monitor();
	serving();

    DataUpdateListener listener;
    listener.addSubscriber(new discordNotifier(cfg, lg, inf));

	DarknetDetector detector(lg, cfg);

	detector.run();
	return 0;
#else

    std::string names_file = cfg.getNamesFile();
    std::string cfg_file = cfg.getCfgFile();
    std::string weights_file = cfg.getWeightFile();
    std::string filename = cfg.getSrc();
    float thresh = cfg.getThreshold();
    auto obj_names = objects_names_from_file(names_file);
    bool const send_network = false;     // true - for remote detection
    bool const use_kalman_filter = true; // true - for stationary camera

    Detector detector(cfg_file, weights_file);
    PidHistoryTracker pid_tracker(15);

    bool detection_sync = true; // true - for video-file

#ifdef TRACK_OPTFLOW // for slow GPU
    detection_sync = false;
    Tracker_optflow tracker_flow;
#endif // TRACK_OPTFLOW

    std::atomic<bool> exit_flag(false);

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
                std::chrono::steady_clock::time_point steady_start, steady_end;
                bool use_zed_camera = false;

                track_kalman_t track_kalman;

                cv::VideoCapture cap;
                if (filename == "web_camera") {
                    cap.open(0);
                    cap >> cur_frame;
                } else if (!use_zed_camera) {
                    cap.open(cfg.getSrc());
                    cap >> cur_frame;
                }

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
                        if (exit_flag) {
                            detection_data.exit_flag = true;
                        } else if (detection_data.cap_frame.empty()) {
                            // verify and polling
                            // here when cam is down
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

                // pre-processing video frame (resize,
                // convertion)
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
                            // get new Detection
                            // result if present
                            if (detect2draw.is_object_present()) {
                                cv::Mat old_cap_frame =
                                  detection_data
                                    .cap_frame; // use old captured frame
                                detection_data = detect2draw.receive();
                                if (!old_cap_frame.empty())
                                    detection_data.cap_frame = old_cap_frame;
                            }
                            // get new Captured
                            // frame
                            else {
                                std::vector<bbox_t> old_result_vec =
                                  detection_data
                                    .result_vec; // use old detections
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
                            } else {
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

                        bool is_inside_polygon = false;
			int num_is_not_inside_polygon = 0;
                        for (auto& obj : result_vec) {
                            if (obj_names.size() > obj.obj_id) {
                                if (obj_names[obj.obj_id] ==
                                    std::string("person")) {
			            auto centrol_point = cv::Point(obj.x, obj.y + obj.h );
                                    if (in_polygon(
                                          polygonVertices,
                                          centrol_point)) {
					    cv::circle(draw_frame, centrol_point, 5, cv::Scalar(0, 0, 255), -1); 
                                        is_inside_polygon = true;
                                    } else {
                                        pid_tracker.push(obj.track_id);
					num_is_not_inside_polygon++;
					//std::cout << "push " << obj.track_id  << " to history due to centrol poin isn't in polygon " << centrol_point << "\n";
                                    }
                                }
                            }
                        }
                        draw_boxes(draw_frame, result_vec, obj_names,
                                           current_fps_det, current_fps_cap,
                                           cfg.getThreshold());

                        if (is_inside_polygon) {
                            polylines(draw_frame, polygonVertices, true,
                                          cv::Scalar(0, 255, 0), 2);
                            bool person_come_in = true;
                            for (auto& obj : result_vec) {
                                if (pid_tracker.contains(obj.track_id)) {
                                    person_come_in = false;
				    //std::cout << "this id is in tracker "<< obj.track_id << "\n"; 
                                    break;
                                }
                            }
			    //std::cout << "person_come_in " << person_come_in << " " << num_is_not_inside_polygon << "\n";
		            //pid_tracker.dump();
                            if (person_come_in && num_is_not_inside_polygon == 0) {

                                DataResult data_result{
                                  draw_frame, convertBbox2obj(result_vec),
                                  obj_names, current_fps_det};
                                listener.onDataUpdate(data_result);
                            }
                        }

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
                    //std::cout << "sleep wait for next events\n";
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                }
            }
        } catch (std::exception& e) {
            std::cerr << "exception: " << e.what() << "\n";
        } catch (...) { std::cerr << "unknown exception \n"; }
    }

    return 0;
#endif
}
