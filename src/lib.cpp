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
    if (curTime < m_time4rest) return;
    this->updateQueueSize(d.fps);

    std::pair<obj_t, std::time_t> p = std::make_pair(d.objs[0], curTime);
    recent_results.push_front(p);

    if (recent_results.size() >= m_queueEntryLimit) {
        // main checking
        auto lastOneTime = recent_results.back().second;
        if (curTime - lastOneTime <= cfg.getTimeForcus()) {
            cv::imwrite(m_tmpImgPath, d.frame);
            m_time4rest = curTime + cfg.getTimeSkippingDetection();
            recent_results.clear();
            logger.info("wrote an image, start skiping ");
            inf.notify(NOTIFY_TYPE::DETECT_RET, m_tmpImgPath);
        }
        else {
            recent_results.pop_back();
        }
    }
}
