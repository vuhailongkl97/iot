#include "server.h"
#include "crow.h"
#include "config_mgr.h"

class CustomLogger : public crow::ILogHandler
{
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

CrowServer::CrowServer(Config& c, Logger& l) : pimpl(new impl), Interface(c, l)
{}

Interface& CrowServer::getInstance(Config& c, Logger& l)
{
    static CrowServer cr(c, l);
    return cr;
}
CrowServer::~CrowServer() {}
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
        os << (ret ? "mok" : "mnok");
        _res.res = crow::response{os.str()};
    }
    return _res;
}

bool CrowServer::initialize()
{
    m_handler = Handler;
    static CustomLogger tmplogger(logger);
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

void CrowServer::notify(NOTIFY_TYPE type, std::string content)
{
    std::string key;
    switch (type) {
        case NOTIFY_TYPE::DETECT_RET: key = "img_path"; break;
        case NOTIFY_TYPE::MSG: key = "msg"; break;
        default: break;
    }
    crow::json::wvalue x;
    x["key"] = key;
    x["content"] = content;
    auto apis = cfg.getNotifyAPI();
    for (auto api : apis) {
        char cmd[250];
        snprintf(cmd, sizeof(cmd) - 1,
                 "/usr/bin/curl %s -X POST -d '%s' --max-time 2 -s >/dev/null",
                 api.c_str(), x.dump().c_str());
        system(cmd);
    }
}
