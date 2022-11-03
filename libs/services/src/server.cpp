#include "server.h"
#include "crow.h"
#include "config_mgr.h"

class CustomLogger : public crow::ILogHandler {
private:
    Logger& m_l;

public:
    explicit CustomLogger(Logger& l) : m_l(l) {}
    void log(std::string message, crow::LogLevel) { m_l.info(message.c_str()); }
};

struct CrowServer::impl
{
    crow::SimpleApp app;
};

CrowServer::CrowServer(Config& c, Logger& l) :
  pimpl(new CrowServer::impl), Interface(c, l)
{}

struct req_impl
{
    const crow::request* req;
};

struct res_impl
{
    crow::response res;
};

res_impl Handler(Config& cfg, req_impl _req)
{
    auto req = _req.req;
    res_impl _res;
    auto x = crow::json::load(req->body);
    if (!x) { _res.res = crow::response(crow::status::BAD_REQUEST); }
    else {
        bool ret = cfg.update(req->body);
        std::ostringstream os;
        os << (ret ? "ok" : "nok");
        _res.res = crow::response{os.str()};
    }
    return _res;
}

bool CrowServer::initialize()
{
    m_handler = Handler;
    CustomLogger tmplogger(logger);
    crow::logger::setHandler(&tmplogger);
    CROW_ROUTE(pimpl->app, "/config")
      .methods("POST"_method)([this](const crow::request& req) {
          req_impl _req;
          _req.req = &req;
          auto _res = this->m_handler(cfg, _req);
          return std::move(_res.res);
      });

    return true;
}

void CrowServer::run() { pimpl->app.port(cfg.getHTTPPort()).run(); }

void CrowServer::notify(std::string addr, std::string content)
{
    std::cout << "notify to addr " << addr << "\n";
}
