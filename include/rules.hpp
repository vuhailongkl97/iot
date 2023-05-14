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

bool in_polygon(const std::vector<cv::Point>& polygon, cv::Point point) {
    return pointPolygonTest(polygon, point, true) >= 0;
}

class Rule {
 public:
    virtual bool filter(std::vector<bbox_t>& vec) const = 0;
    virtual ~Rule() {}
};

class ExcludeAreaRule : public Rule {
 public:
    explicit ExcludeAreaRule(const std::vector<cv::Point>& area) :
      area_(area) {}
    void set_area(std::vector<cv::Point>& area) { area_ = area; }

    bool filter(std::vector<bbox_t>& vec) const override {
        std::vector<int> be_remove_index;

        for (int i = 0; i < vec.size(); i++) {
            auto bbox = vec.at(i);
            auto centrol_point =
              cv::Point(bbox.x + bbox.w / 2, bbox.y + bbox.h / 2);
            if (in_polygon(area_, centrol_point)) {
                be_remove_index.push_back(i);
            }
        }

        int n = be_remove_index.size();
        while (n--) {
            vec.erase(std::next(vec.begin(), be_remove_index.at(n)));
        }
    }

 private:
    std::vector<cv::Point> area_;
};

class Hook {
 public:
    void regist(std::unique_ptr<Rule> rule) {
        hook_list_.push_back(std::move(rule));
    }

    void clean(std::unique_ptr<Rule> rule) { hook_list_.clear(); }

    void run(std::vector<bbox_t>& vec) {
        for (int i = 0; i < hook_list_.size(); i++) {
            hook_list_.at(i)->filter(vec);
        }
    }

 private:
    std::vector<std::unique_ptr<Rule>> hook_list_;
};

class AfterDetectHook {
 public:
    AfterDetectHook() { init(); }

    void init() {
        std::unique_ptr<Rule> rule =
          std::unique_ptr<ExcludeAreaRule>(new ExcludeAreaRule(excluded_area));
        hook_.regist(std::move(rule));
    }

	void run(std::vector<bbox_t>& vec) {
		hook_.run(vec);
	}

 private:
    Hook hook_;
    std::vector<cv::Point> excluded_area = {{282, 175},
                                            {316, 148},
                                            {312, 95},
                                            {289, 96}};
};
