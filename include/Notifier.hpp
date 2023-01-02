#include <opencv2/highgui/highgui_c.h> // C
#include <opencv2/imgproc/imgproc_c.h> // C

#include <chrono>
#include <string>
#include <vector>
#include <memory>

#include <opencv2/opencv.hpp> // C++

class Config;
class Logger;
class Interface;

struct obj_t {
    float prob; // confidence - probability that the object was found correctly
    unsigned int obj_id; // class of object - from range [0, classes-1]
    unsigned int
      track_id; // tracking id for video (0 - untracked, 1 - inf - tracked object)
    unsigned int
      frames_counter; // counter of frames on which the object was detected
};

struct DataResult {
    cv::Mat frame;
    std::vector<obj_t> objs;
    std::vector<std::string> names;
    int fps;
};

struct filteredDataResult {
    cv::Mat frame;
    std::vector<obj_t> objs;
    int fps;
};

class Notifier {
 protected:
    Config& cfg;
    Logger& logger;
    Interface& inf;

 public:
    Notifier(Config& c, Logger& l, Interface& i) : cfg(c), logger(l), inf(i) {}
    void work(const DataResult& d);

 protected:
    static std::chrono::milliseconds getCurrentTime();

 private:
    virtual void doWork(const filteredDataResult& d) = 0;
    static filteredDataResult filterPerson(const DataResult& d);
};

class DataUpdateListener {
 public:
    void addSubscriber(Notifier* nf) { m_nfers.emplace_back(nf); }
    void onDataUpdate(DataResult& data) {
        for (auto& nf : m_nfers) {
            nf->work(data);
        }
    }

 private:
    std::vector<std::unique_ptr<Notifier>> m_nfers;
};
