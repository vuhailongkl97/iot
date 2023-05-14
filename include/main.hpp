#pragma once

#include <thread>
#include "yolo_v2_class.hpp"
#include <opencv2/core/version.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio/videoio.hpp>

template<typename T>
class send_one_replaceable_object_t {
    const bool sync;
    std::atomic<T*> a_ptr;

 public:
    void send(T const& _obj) {
        T* new_ptr = new T;
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

void draw_boxes(cv::Mat mat_img, std::vector<bbox_t> result_vec,
                std::vector<std::string> obj_names, int current_det_fps,
                int current_cap_fps, float thresh) {
    for (auto& i : result_vec) {
        cv::Scalar color = obj_id_to_color(i.obj_id);
        if (obj_names.size() > i.obj_id) {
            std::string obj_name = obj_names[i.obj_id];
            cv::rectangle(mat_img, cv::Rect(i.x, i.y, i.w, i.h), color, 1);
            if (i.track_id > 0) obj_name = std::to_string(i.track_id);
            obj_name += "-";

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
              color, 0, 8, 0);
            putText(mat_img, obj_name, cv::Point2f(i.x, i.y - 16),
                    cv::FONT_HERSHEY_COMPLEX_SMALL, 0.6, cv::Scalar(0, 0, 0),
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

std::vector<obj_t> convertBbox2obj(const std::vector<bbox_t>& v) {
    std::vector<obj_t> ret;
    for (auto& elem : v) {
        ret.push_back(
          {elem.prob, elem.obj_id, elem.track_id, elem.frames_counter});
    }
    return ret;
}

