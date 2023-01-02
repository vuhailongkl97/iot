#include <Notifier.hpp>

#include <chrono>
#include <sstream>
#include <deque>
#include <utility>

#include "services/include/logger.h"
#include "services/include/server.h"
#include "services/include/config_mgr.h"

class consoleNotifier final : public Notifier {
 public:
    void doWork(const filteredDataResult& d) override;
};

class discordNotifier final : public Notifier {
 public:
    explicit discordNotifier(Config& c, Logger& l, Interface& i) :
      Notifier(c, l, i) {
        m_time4rest = getCurrentTime();
    }

 private:
    void updateQueueSize(int fps);
    void doWork(const filteredDataResult& d) override;

    std::deque<std::pair<obj_t, std::chrono::milliseconds>> recent_results;
    const char* m_tmpImgPath = "/tmp/img.png";
    size_t m_queueEntryLimit;
    std::chrono::milliseconds m_time4rest;
};
