#pragma once
#include <vector>
#include <chrono>
#include <string>
#include <sstream>
#include <iostream>
#include <cmath>

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
    cv::Mat f;
    std::vector<obj_t> s;
    std::vector<std::string> d;
    uint64_t frameid;
};

class Notifier
{
public:
    void work(const DataResult& d)
    {
        float avg_prob = prepare(d);
        doNotify(tmpImgFilePath, avg_prob);
    }
    virtual void doNotify(const char* pathToFrame, float avg_prob) = 0;
    float prepare(const DataResult& d)
    {
        //std::cout << "saving the image \n";
        return 0;
    }

private:
    const char* tmpImgFilePath = "/tmp/img.png";
};

class consoleNotifier : public Notifier
{
public:
    void doNotify(const char* pathToFrame, float avg_prob);
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
