#include <chrono>
#include <string>
#include <sstream>
#include <iostream>
#include <cmath>
#include "services/include/logger.h"

#ifdef OPENCV
#include <opencv2/opencv.hpp>          // C++
#include <opencv2/highgui/highgui_c.h> // C
#include <opencv2/imgproc/imgproc_c.h> // C
#endif
#include <fstream>

struct obj_t
{
    float prob;                  // confidence - probability that the object was found correctly
    unsigned int obj_id;         // class of object - from range [0, classes-1]
    unsigned int track_id;       // tracking id for video (0 - untracked, 1 - inf - tracked object)
    unsigned int frames_counter; // counter of frames on which the object was detected
};

struct DataResult
{
    cv::Mat frame;
    std::vector<obj_t> objs;
    std::vector<std::string> names;
};

struct filteredDataResult
{
    cv::Mat frame;
    std::vector<obj_t> objs;
};

class Notifier
{
public:
    void work(const DataResult& d)
    {
        filteredDataResult f_d = filterPerson(d);
        if (!f_d.objs.empty())
            doWork(f_d);
    }
    virtual void doWork(const filteredDataResult& d) = 0;

    filteredDataResult filterPerson(const DataResult& d)
    {
        filteredDataResult filtered_d;
        for (auto& obj : d.objs) {
            if (d.names.size() > obj.obj_id) {
                if (d.names[obj.obj_id] == std::string("person")) {
                    if (filtered_d.frame.empty())
                        filtered_d.frame = d.frame;
                    filtered_d.objs.push_back(obj);
                }
            }
        }
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
    discordNotifier() :
      Notifier()
    {
        time4rest = std::time(nullptr);
    }
    void doWork(const filteredDataResult& d) override;

private:
    std::deque<std::pair<obj_t, std::time_t>> recent_results;
    const char* tmpImgPath = "/tmp/img.png";
    const size_t QUEUE_ENTRY_LIMIT = 20; // maximum queue entry 20
    const int TIME_LIMIT = 2;            // from queue.front().time - queue.back().time <= 2secs
    const int TIME_SKIP = 20;            // 20 sec to prevent spam discord server
    std::time_t time4rest;
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
