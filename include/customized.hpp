#pragma once
#include <string>
#include <list>
#include <deque>
#include <atomic>

#include <opencv2/core/version.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio/videoio.hpp>
#include "main.hpp"
#define TRACK_OPTFLOW

void customizedFrame(cv::Mat& f) {
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
std::vector<cv::Point> polygonVertices = {{404, 236},
                                          {275, 171},
                                          {273, 39},
                                          {415, 78},
                                          {405, 236}};

bool in_polygon(const std::vector<cv::Point>& polygon, cv::Point point) {
    return pointPolygonTest(polygonVertices, point, true) >= 0;
}

class PidHistoryTracker {
 public:
    PidHistoryTracker(size_t max_size) { max_size_ = max_size; }

    void push(uint32_t pid) {
        if (!pids_.size()) {
            pids_.push_back(pid);
        } else {
            if (pids_.back() == pid) return;
            if (max_size_ == size()) { pids_.pop_front(); }
            if (!contains(pid)) pids_.push_back(pid);
        }
    }
    bool contains(uint32_t pid) {
        for (auto it : pids_) {
            if (it == pid) { return true; }
        }
        return false;
    }
    void dump() {
        for (auto it : pids_) {
            std::cout << it << " ";
        }
        std::cout << "\n";
    }

    size_t size() { return pids_.size(); }

 private:
    size_t max_size_;
    std::deque<uint32_t> pids_;
};


/*
 * this class use for creating a hide block used darknet detector it support
 * get status, get current frames, follow multiple source at same time.
 * */
class DarknetDetector {
 public:
    explicit DarknetDetector(Logger&, Config&);
    bool subscribe(Notifier*);
    std::string getStatus();
    void run() {

        auto cap_ = std::async(std::launch::async, [this]() { capture(); });

        auto prepare_ = std::async(std::launch::async, [this]() { prepare(); });

        auto detect_ = std::async(std::launch::async, [this]() { detect(); });

        auto draw_ = std::async(std::launch::async, [this]() { draw(); });

		show();

        cap_.get();
        prepare_.get();
        detect_.get();
        draw_.get();
    }

 private:
    bool capture() {

        if (videoSources.empty()) { return false; }
        cv::VideoCapture cap;
        auto src_path = config.getSrc();
        cap.open(src_path);
        cv::Mat cur_frame;
        cap >> cur_frame;
        if (cur_frame.empty()) { return false; }

        customizedFrame(cur_frame);
        frame_size = cur_frame.size();
        std::cout << "\n Video size: " << frame_size << std::endl;
        if (src_path.find("rtsp") != std::string::npos) {
            detection_sync = false;
        }

        //const bool sync = detection_sync; // sync data exchange

        uint64_t frame_id = 0;
        detection_data_t detection_data;
        do {
            detection_data = detection_data_t();
            cap >> detection_data.cap_frame;
            customizedFrame(detection_data.cap_frame);
            detection_data.frame_id = frame_id++;

            exit_flag = config.status();
            if (exit_flag) {
                detection_data.exit_flag = true;
            } else if (detection_data.cap_frame.empty()) {
                // verify and polling
                // here when cam is down
                std::cout << "retrying open " << src_path << "\n";
                while (cap.open(src_path) == false) {
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
              std::chrono::milliseconds(config.getDelay4Cap()));
        } while (!detection_data.exit_flag);
        std::cout << " t_cap exit \n";
        return true;
    }

    bool prepare() {
        std::shared_ptr<image_t> det_image;
        detection_data_t detection_data;
        do {
            detection_data = cap2prepare.receive();

            det_image = darknet->mat_to_image_resize(detection_data.cap_frame);
            detection_data.det_image = det_image;
            prepare2detect.send(detection_data); // detection

        } while (!detection_data.exit_flag);

        return true;
    }

    bool detect() {
        std::shared_ptr<image_t> det_image;
        detection_data_t detection_data;
        do {
            detection_data = prepare2detect.receive();
            det_image = detection_data.det_image;
            std::vector<bbox_t> result_vec;

            if (det_image)
                result_vec = darknet->detect_resized(
                  *det_image, frame_size.width, frame_size.height,
                  config.getThreshold(),
                  true); // true
            fps_det_counter++;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));

            detection_data.new_detection = true;
            detection_data.result_vec = result_vec;
            detect2draw.send(detection_data);
        } while (!detection_data.exit_flag);
        std::cout << " t_detect exit \n";
        return true;
    }

    bool draw() {
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
                } else {
                    std::vector<bbox_t> old_result_vec =
                      detection_data.result_vec; // use old detections
                    detection_data = cap2draw.receive();
                    detection_data.result_vec = old_result_vec;
                }
            }

            cv::Mat cap_frame = detection_data.cap_frame;
            cv::Mat draw_frame = detection_data.cap_frame.clone();
            std::vector<bbox_t> result_vec = detection_data.result_vec;
#ifdef TRACK_OPTFLOW
            if (detection_data.new_detection) {
                tracker_flow.update_tracking_flow(detection_data.cap_frame,
                                                  detection_data.result_vec);
                while (track_optflow_queue.size() > 0) {
                    draw_frame = track_optflow_queue.back();
                    result_vec = tracker_flow.tracking_flow(
                      track_optflow_queue.front(), false);
                    track_optflow_queue.pop();
                }
            } else {
                track_optflow_queue.push(cap_frame);
                result_vec = tracker_flow.tracking_flow(cap_frame, false);
            }
            detection_data.new_detection = true; // to correct kalman filter
#endif                                           //TRACK_OPTFLOW

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
                  darknet->tracking_id(result_vec, true, frame_story, 40);
            }

            bool is_inside_polygon = false;
            int num_is_not_inside_polygon = 0;
            for (auto& obj : result_vec) {
                if (obj_names.size() > obj.obj_id) {
                    if (obj_names[obj.obj_id] == std::string("person")) {
                        auto centrol_point = cv::Point(obj.x, obj.y + obj.h);
                        if (in_polygon(polygonVertices, centrol_point)) {
                            cv::circle(draw_frame, centrol_point, 5,
                                       cv::Scalar(0, 0, 255), -1);
                            is_inside_polygon = true;
                        } else {
                            pid_tracker.push(obj.track_id);
                            num_is_not_inside_polygon++;
                            //std::cout << "push " << obj.track_id  << " to history due to centrol poin isn't in polygon " << centrol_point << "\n";
                        }
                    }
                }
            }

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

                    DataResult data_result{draw_frame,
                                           convertBbox2obj(result_vec),
                                           obj_names, current_fps_det};
                    listener.onDataUpdate(data_result);
                }
            }

            draw_boxes(draw_frame, result_vec, obj_names, current_fps_det,
                       current_fps_cap, config.getThreshold());

            detection_data.result_vec = result_vec;
            detection_data.draw_frame = draw_frame;
            steady_end = std::chrono::steady_clock::now();
            float time_sec =
              std::chrono::duration<double>(steady_end - steady_start).count();
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
        } while (!detection_data.exit_flag);
        std::cout << " t_draw exit \n";
        return true;
    }
	void show() {
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

    }

 private:
    send_one_replaceable_object_t<detection_data_t> cap2prepare, cap2draw,
      prepare2detect, detect2draw, draw2show, draw2write, draw2net;
    std::vector<std::string> videoSources;
    bool const use_kalman_filter = true; // true - for stationary camera
    std::unique_ptr<Detector> darknet;
    Logger& logger;
    Config& config;
    bool detection_sync = false;
    DataUpdateListener listener;
    bool exit_flag = false;
    cv::Size frame_size;

    // fps monitoring
    std::atomic<int> fps_cap_counter, fps_det_counter;
    std::atomic<int> current_fps_cap, current_fps_det;
    std::chrono::steady_clock::time_point steady_start, steady_end;

    // trackerflow
#ifdef TRACK_OPTFLOW
    Tracker_optflow tracker_flow;
#endif
    track_kalman_t track_kalman;

    std::vector<std::string> obj_names;

    PidHistoryTracker pid_tracker;

    using Hook = std::function<void(void)>;
    //a hook function list.
    std::list<Hook> hook_captures;
    std::list<Hook> hook_prepare;
    std::list<Hook> hook_detect;
    std::list<Hook> hook_draw;
};

DarknetDetector::DarknetDetector(Logger& _lg, Config& _cfg) :
  cap2prepare(false), cap2draw(false), prepare2detect(false),
  detect2draw(false), draw2show(false), draw2write(false), draw2net(false),
  logger(_lg), config(_cfg), fps_cap_counter(0), fps_det_counter(0),
  current_fps_cap(0), current_fps_det(0), pid_tracker(15) {
    darknet.reset(new Detector(config.getCfgFile(), config.getWeightFile()));
    videoSources.push_back(config.getSrc());
    auto names_file = config.getNamesFile();
    obj_names = objects_names_from_file(names_file);
}

/*
 * subscribe events when detector got a detected results
 * @param Notifier *
 * */

bool DarknetDetector::subscribe(Notifier* ntf) {
    bool result = true;

    listener.addSubscriber(ntf);

    return result;
}

