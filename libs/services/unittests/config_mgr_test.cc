#include <gtest/gtest.h>

#include "../src/config_mgr.cpp"

TEST(JSONTEST, DEFAULT_CFG)
{
    Config& cfg = JSONConfig::getInstance("../iot-config.json");

    EXPECT_TRUE(cfg.status() == false);
    EXPECT_TRUE(cfg.getTimeForcus() == 2);
    EXPECT_TRUE(cfg.getTimeSkippingDetection() == 20);
    EXPECT_TRUE(cfg.getMinQueueEntryLimit() == 15);
	auto apis = cfg.getNotifyAPI();
	ASSERT_EQ(apis.size(), 2);
    EXPECT_EQ(apis[0], std::string("http://localhost:1234/updated"));
    EXPECT_EQ(apis[1], std::string("abc:9090"));
    EXPECT_TRUE(cfg.getDelay4Cap() == 20);
    EXPECT_TRUE(cfg.getHTTPPort() == 18080);
    EXPECT_TRUE(cfg.getNamesFile() == std::string("coco.names"));
    EXPECT_TRUE(cfg.getCfgFile() == std::string("yolov4-tiny.cfg"));
    EXPECT_TRUE(cfg.getWeightFile() == std::string("yolov4-tiny.weights"));
    EXPECT_TRUE(fabs(cfg.getThreshold() - 0.9) < 0.001);
    cfg.show();
}

TEST(JSONTEST, SET_THRESHOLD)
{
    Config& cfg = JSONConfig::getInstance("../iot-config.json");

    auto tmpval = cfg.getThreshold();
    cfg.setThreshold(0.12);
    EXPECT_TRUE(fabs(cfg.getThreshold() - 0.12) < 0.001);

    cfg.setThreshold(tmpval);
    EXPECT_TRUE(fabs(cfg.getThreshold() - tmpval) < 0.001);
}

TEST(JSONTEST, PARSE) {
    using json = nlohmann::json;

    Config& cfg = JSONConfig::getInstance("../iot-config.json");
	json ex1 = {
	  {"NotifyAPI", {"https://", "admins:8080"}},
	  {"Status", false}
	};

	cfg.parse(ex1.dump());

	auto apis = cfg.getNotifyAPI();

	ASSERT_EQ(apis.size(), 2);
    EXPECT_EQ(apis[0], std::string("https://"));
    EXPECT_EQ(apis[1], std::string("admins:8080"));

	json ex2 = {
	  {"NotifyAPI", {"https://"}},
	  {"Status", false}
	};
	cfg.parse(ex2.dump());
	apis = cfg.getNotifyAPI();

	ASSERT_EQ(apis.size(), 1);
    EXPECT_EQ(apis[0], std::string("https://"));
}
