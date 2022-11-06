#include <chrono>
#include <string>
#include <sstream>
#include <iostream>
#include <cmath>
#include "services/include/logger.h"
#include "services/include/server.h"
#include "services/include/config_mgr.h"

#ifdef OPENCV
#include <opencv2/opencv.hpp>          // C++
#include <opencv2/highgui/highgui_c.h> // C
#include <opencv2/imgproc/imgproc_c.h> // C
#endif
#include <fstream>

struct obj_t
{
    float prob; // confidence - probability that the object was found correctly
    unsigned int obj_id; // class of object - from range [0, classes-1]
    unsigned int
      track_id; // tracking id for video (0 - untracked, 1 - inf - tracked object)
    unsigned int
      frames_counter; // counter of frames on which the object was detected
};

struct DataResult
{
    cv::Mat frame;
    std::vector<obj_t> objs;
    std::vector<std::string> names;
    int fps;
};

struct filteredDataResult
{
    cv::Mat frame;
    std::vector<obj_t> objs;
    int fps;
};

class Notifier
{
protected:
    Config& cfg;
    Logger& logger;
    Interface& inf;

public:
    Notifier(Config& c, Logger& l, Interface& i) : cfg(c), logger(l), inf(i) {}
    void work(const DataResult& d)
    {
        filteredDataResult f_d = filterPerson(d);
        if (!f_d.objs.empty()) doWork(f_d);
    }
    virtual void doWork(const filteredDataResult& d) = 0;

    filteredDataResult filterPerson(const DataResult& d)
    {
        filteredDataResult filtered_d;
        for (auto& obj : d.objs) {
            if (d.names.size() > obj.obj_id) {
                if (d.names[obj.obj_id] == std::string("person")) {
                    if (filtered_d.frame.empty()) filtered_d.frame = d.frame;
                    filtered_d.objs.push_back(obj);
                }
            }
        }
        filtered_d.fps = d.fps;
        return filtered_d;
    }
};

class consoleNotifier final : public Notifier
{
public:
    void doWork(const filteredDataResult& d) override;
};

class discordNotifier final : public Notifier
{
public:
    explicit discordNotifier(Config& c, Logger& l, Interface& i) :
      Notifier(c, l, i)
    {
        m_time4rest = std::time(nullptr);
    }

    void updateQueueSize(int fps)
    {
        m_queueEntryLimit =
          fps * (cfg.getTimeForcus() + 1) * cfg.getCorrectRate();
        if (m_queueEntryLimit < cfg.getMinQueueEntryLimit())
            m_queueEntryLimit = cfg.getMinQueueEntryLimit();
    }
    void doWork(const filteredDataResult& d) override;

private:
    std::deque<std::pair<obj_t, std::time_t>> recent_results;
    const char* m_tmpImgPath = "/tmp/img.png";
    size_t m_queueEntryLimit;
    std::time_t m_time4rest;
};

class DataUpdateListener
{
public:
    void addSubscriber(Notifier* nf) { m_nfers.emplace_back(nf); }
    void onDataUpdate(DataResult& data)
    {
        for (auto& nf : m_nfers) {
            nf->work(data);
        }
    }

private:
    std::vector<std::unique_ptr<Notifier>> m_nfers;
};
