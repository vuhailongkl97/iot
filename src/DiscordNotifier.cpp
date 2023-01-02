#include "DiscordNotifier.hpp"

using std::chrono::duration;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::duration_cast;

void show_console_result(std::vector<obj_t> const result_vec) {
    for (auto& i : result_vec) {
        std::cout << "obj_id = " << i.obj_id << std::setprecision(3)
                  << ", prob = " << i.prob << std::endl;
    }
}

NotifyState consoleNotifier::doWork(const filteredDataResult& d) {
    show_console_result(d.objs);
    return NotifyState::UPDATED;
}

NotifyState discordNotifier::doWork(const filteredDataResult& d) {
    auto curTime = getCurrentTime();
    if (curTime < m_time4rest) return NotifyState::LOCKING;
    this->updateQueueSize(d.fps);

    std::pair<obj_t, milliseconds> p = std::make_pair(d.objs[0], curTime);
    recent_results.push_front(p);

    if (recent_results.size() >= m_queueEntryLimit) {
        // main checking
        auto lastOneTime = recent_results.back().second;
        seconds skippingTime(cfg.getTimeSkippingDetection());
        milliseconds timeForcus(cfg.getTimeForcus());
        if (curTime - lastOneTime <= timeForcus) {
            cv::imwrite(m_tmpImgPath, d.frame);
            m_time4rest = curTime + skippingTime;
            recent_results.clear();
            logger.info("wrote an image, start skiping ");
            inf.notify(NOTIFY_TYPE::DETECT_RET, m_tmpImgPath);
            return NotifyState::UPDATED;
        } else {
            recent_results.pop_back();
        }
    }
    return NotifyState::CHECKING;
}

void discordNotifier::updateQueueSize(int fps) {
    milliseconds timeForcus(cfg.getTimeForcus());
    m_queueEntryLimit =
      fps * duration_cast<duration<double>>(timeForcus).count() *
      cfg.getCorrectRate();
    if (m_queueEntryLimit < cfg.getMinQueueEntryLimit())
        m_queueEntryLimit = cfg.getMinQueueEntryLimit();
}
