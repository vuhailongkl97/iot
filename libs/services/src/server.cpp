#include "server.h"
#include "crow.h"

void runServer(float &threshVal) {
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
  app.port(18080).run();
}
