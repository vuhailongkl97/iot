#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

class MLogger
{

private:
    MLogger() = delete;
    static void initializeLog();

public:
    static std::shared_ptr<spdlog::logger> getLoggerInstance();
};
