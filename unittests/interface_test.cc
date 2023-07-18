#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/server.cpp"

class MockSystem {
 public:
    MOCK_METHOD(int, system, (const char*));
};

MockSystem* _m = nullptr;
int system(const char* command) {
    if (_m) { return _m->system(command); }
    return 0;
}

class INTERFACETest : public ::testing::Test {
 public:
    INTERFACETest() :
      lg(spdLogger().getInstance()),
      cfg(JSONConfig::getInstance("../iot-config.json")),
      p(new CrowServer(cfg, lg)) {}

 protected:
    void SetUp() noexcept override { _m = new MockSystem(); }

    void TearDown() override { delete _m; }
    Logger& lg;
    Config& cfg;
    std::unique_ptr<Interface> p;
};

TEST_F(INTERFACETest, RETURN400) {
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

TEST_F(INTERFACETest, UPDATE) {
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
			"NotifyAPI": ["http://localhost:1234/updated", "https://localhost:12"],
			"Port": 18080,
			"QUEUE_ENTRY_LIMIT_MIN": 15,
			"Src": "rtsp://admin:admin@192.168.1.3:554/cam/realmonitor?channel=4&subtype=1",
			"TIME_FORCUS": 2,
			"TIME_SKIP": 20,
			"Threshold": 0.9,
			"WeightFile": "yolov4-tiny.weights"
		})";

        req_impl _req;
        _req.req = &req;
        auto tmpval = cfg.getThreshold();
        auto _res = p->getHandler()(cfg, _req);
        EXPECT_EQ(_res.res.code, 200);
        std::cout << _res.res.body;
        cfg.show();
        cfg.setThreshold(tmpval);
    }
    using ::testing::InSequence;
    {
        const char* expected_str_req =
          "/usr/bin/curl http://localhost:1234/updated -X POST -H "
          "'Content-Type: application/json' -d "
          "'{\"content\":\"abcdef\",\"key\":\"img_path\"}' --max-time 1 -s "
          ">/dev/null";

        const char* expected_str_req2 =
          "/usr/bin/curl https://localhost:12 -X POST -H 'Content-Type: "
          "application/json' -d "
          "'{\"content\":\"abcdef\",\"key\":\"img_path\"}' --max-time 1 -s "
          ">/dev/null";
        EXPECT_CALL(*_m, system(testing::StrEq(expected_str_req))).Times(1);
        EXPECT_CALL(*_m, system(testing::StrEq(expected_str_req2))).Times(1);

        p->notify(NOTIFY_TYPE::DETECT_RET, "abcdef");
    }
}

TEST_F(INTERFACETest, TEST_ARRAY_NOT_DUPLICATE) {
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
			"Threshold": 0.9,
			"WeightFile": "yolov4-tiny.weights"
		})";

        req_impl _req;
        _req.req = &req;
        auto tmpval = cfg.getThreshold();
        auto _res = p->getHandler()(cfg, _req);
        EXPECT_EQ(_res.res.code, 200);
        std::cout << _res.res.body;
        cfg.show();
        EXPECT_TRUE(fabs(cfg.getThreshold() - 0.9) < 0.001);
        cfg.setThreshold(tmpval);
    }
    using ::testing::InSequence;
    {
        const char* expected_str_req =
          "/usr/bin/curl http://localhost:1234/updated -X POST -H "
          "'Content-Type: application/json' -d "
          "'{\"content\":\"abcdef\",\"key\":\"img_path\"}' --max-time 1 -s "
          ">/dev/null";

        const char* expected_str_req2 =
          "/usr/bin/curl https://localhost:12 -X POST -H 'Content-Type: "
          "application/json' -d "
          "'{\"content\":\"abcdef\",\"key\":\"img_path\"}' --max-time 1 -s "
          ">/dev/null";
        EXPECT_CALL(*_m, system(testing::StrEq(expected_str_req))).Times(1);
        EXPECT_CALL(*_m, system(testing::StrEq(expected_str_req2))).Times(1);

        p->notify(NOTIFY_TYPE::DETECT_RET, "abcdef");
    }
}

TEST_F(INTERFACETest, TEST_ARRAY) {
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
			"NotifyAPI": ["http://localhost:1234/updated"],
			"Port": 18080,
			"QUEUE_ENTRY_LIMIT_MIN": 15,
			"Src": "rtsp://admin:admin@192.168.1.3:554/cam/realmonitor?channel=4&subtype=1",
			"TIME_FORCUS": 2,
			"TIME_SKIP": 20,
			"Threshold": 0.9,
			"WeightFile": "yolov4-tiny.weights"
		})";

        req_impl _req;
        _req.req = &req;
        auto tmpval = cfg.getThreshold();
        auto _res = p->getHandler()(cfg, _req);
        EXPECT_EQ(_res.res.code, 200);
        std::cout << _res.res.body;
        cfg.show();
        EXPECT_TRUE(fabs(cfg.getThreshold() - 0.9) < 0.001);
        cfg.setThreshold(tmpval);
    }
    const char* expected_str_req =
      "/usr/bin/curl http://localhost:1234/updated -X POST -H 'Content-Type: "
      "application/json' -d "
      "'{\"content\":\"abcdef\",\"key\":\"img_path\"}' --max-time 1 -s "
      ">/dev/null";
    EXPECT_CALL(*_m, system(testing::StrEq(expected_str_req))).Times(1);
    p->notify(NOTIFY_TYPE::DETECT_RET, "abcdef");
}
