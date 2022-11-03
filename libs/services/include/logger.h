#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

enum class LOGLV {
    INFO,
    WARNING,
    ERROR
};
class Logger {
 protected:
	LOGLV m_loglevel = LOGLV::INFO;
 public:
	virtual void info(const char *str) = 0;
	virtual void error(const char *str) = 0;
	virtual Logger& getInstance() = 0;
};

class spdLogger final : public Logger {
 private:
	void initialze();
 public:
	 void info(const char *str); 
	 void error(const char *str); 
	 Logger& getInstance(); 
	 std::shared_ptr<spdlog::logger> getLoggerInstance();
};
