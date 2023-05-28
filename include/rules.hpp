#pragma once

#include "yolo_v2_class.hpp" // imported functions from DLL

#ifdef OPENCV
#include <opencv2/core/version.hpp>
#include <opencv2/opencv.hpp> // C++
#ifndef CV_VERSION_EPOCH      // OpenCV 3.x and 4.x
#include <opencv2/videoio/videoio.hpp>
#define OPENCV_VERSION          \
    CVAUX_STR(CV_VERSION_MAJOR) \
    "" CVAUX_STR(CV_VERSION_MINOR) "" CVAUX_STR(CV_VERSION_REVISION)
#ifndef USE_CMAKE_LIBS
#pragma comment(lib, "opencv_world" OPENCV_VERSION ".lib")
#endif // USE_CMAKE_LIBS
#else  // OpenCV 2.x
#define OPENCV_VERSION          \
    CVAUX_STR(CV_VERSION_EPOCH) \
    "" CVAUX_STR(CV_VERSION_MAJOR) "" CVAUX_STR(CV_VERSION_MINOR)
#ifndef USE_CMAKE_LIBS
#pragma comment(lib, "opencv_core" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_imgproc" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_highgui" OPENCV_VERSION ".lib")
#pragma comment(lib, "opencv_video" OPENCV_VERSION ".lib")
#endif // USE_CMAKE_LIBS
#endif // CV_VERSION_EPOCH

#endif // OPENCV

#include "server.h"
#include "config_mgr.h"
#include "pid_tracker.hpp"

std::vector<std::string> objects_names_from_file(std::string const filename);

bool in_polygon(const std::vector<cv::Point>& polygon, cv::Point point) {
    return pointPolygonTest(polygon, point, true) >= 0;
}

class Rule {
 public:
    virtual bool filter(std::vector<bbox_t>& vec) = 0;
    virtual ~Rule() {}
};

class ExcludeAreaRule : public Rule {
 public:
    explicit ExcludeAreaRule(const std::vector<cv::Point>& area) :
      area_(area) {}
    void set_area(std::vector<cv::Point>& area) { area_ = area; }

    bool filter(std::vector<bbox_t>& vec) override {
        for (auto it = vec.begin(); it != vec.end();) {
            auto bbox = *it;
            auto centrol_point =
              cv::Point(bbox.x + bbox.w / 2, bbox.y + bbox.h / 2);
            if (in_polygon(area_, centrol_point)) {
                vec.erase(it);
            } else {
                it++;
            }
        }
        return true;
    }

 private:
    std::vector<cv::Point> area_;
};

struct MetaData {};

struct Data : public MetaData {};

class FilterPersonRule : public Rule {
 public:
    explicit FilterPersonRule(std::vector<std::string> obj_names) :
      obj_names_(obj_names) {}

    bool filter(std::vector<bbox_t>& vec) override {
        for (auto it = vec.begin(); it != vec.end();) {
            auto obj = *it;
            if (obj_names_.size() > obj.obj_id) {
                if (obj_names_[obj.obj_id] != std::string("person")) {
                    vec.erase(it);
                } else {
                    it++;
                }
            } else {
                it++;
            }
        }
        return true;
    }

 private:
    std::vector<std::string> obj_names_;
};

// checked area is used for check a person come in or go out
class CrossAreaRule : public Rule {
 public:
    explicit CrossAreaRule(const std::vector<cv::Point>& area) :
      pid_tracker(15), area_(area) {}
    void set_area(std::vector<cv::Point>& area) { area_ = area; }

    bool filter(std::vector<bbox_t>& vec) override {
        bool is_inside_polygon = false;
        for (auto& obj : vec) {
            auto centrol_point = cv::Point(obj.x, obj.y + obj.h);
            if (in_polygon(area_, centrol_point)) {
                is_inside_polygon = true;
            } else {
                pid_tracker.push(obj.track_id);
            }
        }

        if (is_inside_polygon) {
            bool person_come_in = true;
            for (auto it = vec.begin(); it != vec.end(); it++) {
                auto obj = *it;
                if (pid_tracker.contains(obj.track_id)) {
                    person_come_in = false;
                }
                if (person_come_in) {
                    if (time(0) - pid_tracker.recent_active() >= 3) {
                        return true;
                    }
                }
            }
        }
        vec.clear();
        return true;
    }

 private:
    PidHistoryTracker pid_tracker;
    std::vector<cv::Point> area_;
};

class Hook {
 public:
    void regist(std::unique_ptr<Rule> rule) {
        hook_list_.push_back(std::move(rule));
    }

    void clean() { hook_list_.clear(); }

    void run(std::vector<bbox_t>& vec,
             std::function<void(int i, std::vector<bbox_t>)> callback =
               std::function<void(int, std::vector<bbox_t>)>()) {
        for (int i = 0; i < hook_list_.size(); i++) {
            hook_list_.at(i)->filter(vec);
            if (callback) { callback(i, vec); }
        }
    }

 private:
    std::vector<std::unique_ptr<Rule>> hook_list_;
};

class AfterDetectHook {
 public:
    AfterDetectHook() { init(); }

    void init() {
        auto& cfg = JSONConfig::getInstance("/etc/iot-config.json");
        auto obj_names = objects_names_from_file(cfg.getNamesFile());

        std::unique_ptr<Rule> rule =
          std::unique_ptr<FilterPersonRule>(new FilterPersonRule(obj_names));
        hook_.regist(std::move(rule));

        rule = std::unique_ptr<CrossAreaRule>(new CrossAreaRule(cross_area));
        hook_.regist(std::move(rule));

        rule =
          std::unique_ptr<ExcludeAreaRule>(new ExcludeAreaRule(excluded_area));

        hook_.regist(std::move(rule));
    }

    void run(std::vector<bbox_t>& vec,
             std::function<void(int i, std::vector<bbox_t>)> callback =
               std::function<void(int, std::vector<bbox_t>)>()) {
        hook_.run(vec, callback);
    }

    void draw(cv::Mat& draw_frame) {
        polylines(draw_frame, excluded_area, true, cv::Scalar(0, 255, 0), 2);
        polylines(draw_frame, cross_area, true, cv::Scalar(0, 160, 0), 2);
    }

    friend class HookTest;

 private:
    Hook hook_;
    std::vector<cv::Point> excluded_area = {{282, 175},
                                            {316, 148},
                                            {312, 95},
                                            {289, 96}};

    std::vector<cv::Point> cross_area = {{408, 234},
                                         {290, 179},
                                         {347, 122},
                                         {414, 135}};
};
