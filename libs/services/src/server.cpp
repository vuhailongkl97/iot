#include "server.h"
#include "crow.h"
class CustomLogger : public crow::ILogHandler {
 public:
  CustomLogger() {}
  void log(std::string message, crow::LogLevel /*level*/) {
    // "message" doesn't contain the timestamp and loglevel
    // prefix the default logger does and it doesn't end
    // in a newline.
    //std::cerr << message << std::endl;
  }
};

void runServer(float& threshVal, std::atomic<bool>& exit_flag)
{
    CustomLogger logger;
    crow::logger::setHandler(&logger);
    crow::SimpleApp app;
    app.concurrency(1);
    CROW_ROUTE(app, "/threshold/<int>")
    ([&threshVal](int threshold) {
        if (threshold > 100 || threshold < 0)
            return crow::response(400);
        threshVal = threshold / 100.0;
        std::ostringstream os;
        os << "setting successfully";
        return crow::response(os.str());
    });

    CROW_ROUTE(app, "/disable/<int>")
    ([&exit_flag](int _exit_flag) {
        exit_flag = (int)_exit_flag;
        std::ostringstream os;
        os << "setting" << exit_flag << " successfully";
        return crow::response(os.str());
    });
    app.port(18080).run();
}
