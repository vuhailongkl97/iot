#include "lib.h"

void show_console_result(std::vector<obj_t> const result_vec)
{
    for (auto& i : result_vec) {
        std::cout << "obj_id = " << i.obj_id << std::setprecision(3)
                  << ", prob = " << i.prob << std::endl;
    }
}

void consoleNotifier::doWork(const filteredDataResult& d)
{
    show_console_result(d.objs);
}

void discordNotifier::doWork(const filteredDataResult& d)
{
    auto curTime = std::time(nullptr);
    if (curTime < time4rest) return;

    std::pair<obj_t, std::time_t> p = std::make_pair(d.objs[0], curTime);
    recent_results.push_front(p);

    if (recent_results.size() >= QUEUE_ENTRY_LIMIT) {
        // main checking
        auto lastOneTime = recent_results.back().second;
        if (curTime - lastOneTime <= TIME_LIMIT) {
            cv::imwrite(tmpImgPath, d.frame);
            time4rest = curTime + TIME_SKIP;
            recent_results.clear();
            MLogger::getLoggerInstance()->info(
              "wrote an image, start skiping for {} secs ", TIME_SKIP);
            system("/usr/bin/curl http://localhost:1234/updated -X POST -d "
                   "\"/tmp/img.png\" --max-time 2 -s >/dev/null");
        }
        else {
            recent_results.pop_back();
        }
    }
}
