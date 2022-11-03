#include <gtest/gtest.h>

#include "../src/config_mgr.cpp"

// Demonstrate some basic assertions.
TEST(YamlConfig, DefaultConfig)
{
    // Expect two strings not to be equal.
    Config& cfg = YamlConfig::getInstance("../iot-config.yaml");
    EXPECT_TRUE(cfg.getTimeForcus() == 2);
    EXPECT_TRUE(cfg.getTimeSkippingDectection() == 20);
    EXPECT_TRUE(cfg.getMinQueueEntryLimit() == 15);
    EXPECT_TRUE(cfg.getNotifyAPI() ==
                std::string("http://localhost:1234/updated"));
    EXPECT_TRUE(cfg.getDelay4Cap() == 20);
    EXPECT_TRUE(cfg.getHTTPPort() == 18080);
    EXPECT_TRUE(cfg.getNamesFile() == std::string("coco.names"));
    EXPECT_TRUE(cfg.getCfgFile() == std::string("yolov4-tiny.cfg"));
    EXPECT_TRUE(cfg.getWeightFile() == std::string("yolov4-tiny.weights"));
    EXPECT_TRUE(fabs(cfg.getThreshold() - 0.6) < 0.001);
    cfg.sync();
    cfg.show();
}


TEST(JSONTEST, DEFAULT_CFG)
{
    Config& cfg = JSONConfig::getInstance("../iot-config.json");

    EXPECT_TRUE(cfg.getTimeForcus() == 2);
    EXPECT_TRUE(cfg.getTimeSkippingDectection() == 20);
    EXPECT_TRUE(cfg.getMinQueueEntryLimit() == 15);
    EXPECT_TRUE(cfg.getNotifyAPI() ==
                std::string("http://localhost:1234/updated"));
    EXPECT_TRUE(cfg.getDelay4Cap() == 20);
    EXPECT_TRUE(cfg.getHTTPPort() == 18080);
    EXPECT_TRUE(cfg.getNamesFile() == std::string("coco.names"));
    EXPECT_TRUE(cfg.getCfgFile() == std::string("yolov4-tiny.cfg"));
    EXPECT_TRUE(cfg.getWeightFile() == std::string("yolov4-tiny.weights"));
    EXPECT_TRUE(fabs(cfg.getThreshold() - 0.9) < 0.001);
    cfg.sync();
    cfg.show();
}

TEST(JSONTEST, SET_THRESHOLD)
{
    Config& cfg = JSONConfig::getInstance("../iot-config.json");

	auto tmpval = cfg.getThreshold();
	cfg.setThreshold(0.12);
    EXPECT_TRUE(fabs(cfg.getThreshold() - 0.12) < 0.001);
    cfg.sync();

	cfg.setThreshold(tmpval);
    EXPECT_TRUE(fabs(cfg.getThreshold() - tmpval) < 0.001);
	cfg.sync();
}
