#include "server.h"
#include "crow.h"

void runServer(float& threshVal, std::atomic<bool>& exit_flag)
{
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
