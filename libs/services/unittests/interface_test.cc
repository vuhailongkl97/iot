#include <gtest/gtest.h>

#include "../src/server.cpp"

TEST(INTERFACE, RETURN400)
{

    Logger& lg = spdLogger().getInstance();
    Config& cfg = JSONConfig::getInstance("../iot-config.json");
    std::unique_ptr<Interface> p(new CrowServer(cfg, lg));

    p->initialize();
    {
        crow::request req;
        crow::request res;
        req.url = "/config";
        req_impl _req;
        _req.req = &req;
        auto _res = p->getHandler()(cfg, _req);
        EXPECT_EQ(_res.res.code, 400);
    }
}

TEST(INTERFACE, SUCCESS)
{

    Logger& lg = spdLogger().getInstance();
    Config& cfg = JSONConfig::getInstance("../iot-config.json");
    std::unique_ptr<Interface> p(new CrowServer(cfg, lg));

    p->initialize();
    {
        crow::request req;
        crow::request res;
        req.url = "/config";
        req.body = R"(
		{
			"BoardName": "JetsonNano",
			"CfgFile": "yolov4-tiny.cfg",
			"CorrectRate": 0.9,
			"Delay4CAP": 20,
			"NameFile": "coco.names",
			"NotifyAPI": "http://localhost:1234/updated",
			"Port": 18080,
			"QUEUE_ENTRY_LIMIT_MIN": 15,
			"Src": "rtsp://admin:admin@192.168.1.3:554/cam/realmonitor?channel=4&subtype=1",
			"TIME_FORCUS": 2,
			"TIME_SKIP": 20,
			"Threshold": 0.1,
			"WeightFile": "yolov4-tiny.weights"
		})";

        req_impl _req;
        _req.req = &req;
		auto tmpval = cfg.getThreshold();
        auto _res = p->getHandler()(cfg, _req);
        EXPECT_EQ(_res.res.code, 200);
        std::cout << _res.res.body;
        cfg.show();
		EXPECT_TRUE(fabs(cfg.getThreshold() - 0.1) < 0.001);
		cfg.setThreshold(tmpval);
		cfg.sync();
    }
    p->notify(NOTIFY_TYPE::DETECT_RET, "abcdef");
}
