#include <gtest/gtest.h>
#include <gmock/gmock.h>
#define protected public
#include "../include/DiscordNotifier.hpp"
using testing::_;

namespace cv {
bool imwrite( const String& filename, InputArray img,
              const std::vector<int>& params) {
		return false;
	}

};
#include "../src/DiscordNotifier.cpp"

class MockLogger : public Logger {
 public:
    MOCK_METHOD(void, info, (const char* str));
    MOCK_METHOD(void, error, (const char* str));
    MOCK_METHOD(void, debug, (const char* str));
    MOCK_METHOD(void, warn, (const char* str));
};

class MockInterface : public Interface {
 public:
    MockInterface(Config& cfg, Logger& log) : Interface(cfg, log) {}
    MOCK_METHOD(bool, initialize, ());
    MOCK_METHOD(void, run, ());
    MOCK_METHOD(void, notify, (NOTIFY_TYPE, std::string content));
};

class MockConfig : public Config {
 public:
    MOCK_METHOD(bool, status, ());
    MOCK_METHOD(bool, parse, (std::string));
    MOCK_METHOD(time_t, getTimeForcus, ());
    MOCK_METHOD(time_t, getTimeSkippingDetection, ());
    MOCK_METHOD(int, getMinQueueEntryLimit, ());
    MOCK_METHOD(std::vector<std::string>, getNotifyAPI, ());
    MOCK_METHOD(int, getDelay4Cap, ());
    MOCK_METHOD(int, getHTTPPort, ());
    MOCK_METHOD(std::string, getNamesFile, ());
    MOCK_METHOD(std::string, getCfgFile, ());
    MOCK_METHOD(std::string, getWeightFile, ());
    MOCK_METHOD(float, getThreshold, ());
    MOCK_METHOD(void, setThreshold, (float));
    MOCK_METHOD(std::string, getSrc, ());
    MOCK_METHOD(float, getCorrectRate, ());
    MOCK_METHOD(std::string, getBoardName, ());
    MOCK_METHOD(void, show, ());
    MOCK_METHOD(void, sync, ());
};

MockConfig* cfg = nullptr;
MockLogger* logger = nullptr;
MockInterface* interface = nullptr;

class DiscordNotifierTest : public ::testing::Test {
 public:
    DiscordNotifierTest() {}

 protected:
    void SetUp() noexcept override {
        cfg = new MockConfig();
        logger = new MockLogger;
        interface = new MockInterface(*cfg, *logger);
        m_ptr.reset(new discordNotifier(*cfg, *logger, *interface));
    }

    void TearDown() override {
        delete cfg;
        delete logger;
        delete interface;
    }

 protected:
    std::unique_ptr<discordNotifier> m_ptr;
};

/*
	when queue limit is 1 = fps * correctRate * timeForcus,
    as soon as a picture from detector -> notify
*/
TEST_F(DiscordNotifierTest, work_updated) {
    DataResult d;
    d.fps = 10;
    d.names.push_back("person");
    d.objs.push_back(obj_t{});
    EXPECT_CALL(*cfg, getCorrectRate()).WillOnce(testing::Return(1));
    EXPECT_CALL(*cfg, getTimeSkippingDetection()).WillOnce(testing::Return(10));
    // 100 ms
    EXPECT_CALL(*cfg, getTimeForcus()).WillRepeatedly(testing::Return(100));
    EXPECT_CALL(*cfg, getMinQueueEntryLimit()).WillOnce(testing::Return(1));
    EXPECT_CALL(*logger, info("wrote an image, start skiping ")).Times(1);
    EXPECT_CALL(*interface, notify(_, _)).Times(1);
    EXPECT_EQ(NotifyState::UPDATED, m_ptr->work(d));
}

/*
	when queue limit is 10 = fps * correctRate * timeForcus,
	when a picture is came -> enqueue -> return checking
*/
TEST_F(DiscordNotifierTest, work_checking) {
    DataResult d;
    d.fps = 10;
    d.names.push_back("person");
    d.objs.push_back(obj_t{});
    EXPECT_CALL(*cfg, getCorrectRate()).WillOnce(testing::Return(1));
    // 100 ms
    EXPECT_CALL(*cfg, getTimeForcus()).WillRepeatedly(testing::Return(1000));
    EXPECT_CALL(*cfg, getMinQueueEntryLimit()).WillOnce(testing::Return(1));
    EXPECT_EQ(NotifyState::CHECKING, m_ptr->work(d));
}

/*
	when queue limit is 1 = fps * correctRate * timeForcus,
    as soon as a picture from detector -> notify -> rest 
then
	another picture came -> system in skipping time -> return LOCKING
*/

TEST_F(DiscordNotifierTest, work_locking) {
    DataResult d;
    d.fps = 10;
    d.names.push_back("person");
    d.objs.push_back(obj_t{});
    EXPECT_CALL(*cfg, getCorrectRate()).WillOnce(testing::Return(1));
    EXPECT_CALL(*cfg, getTimeSkippingDetection()).WillOnce(testing::Return(10));
    // 100 ms
    EXPECT_CALL(*cfg, getTimeForcus()).WillRepeatedly(testing::Return(100));
    EXPECT_CALL(*cfg, getMinQueueEntryLimit()).WillOnce(testing::Return(1));
    EXPECT_CALL(*logger, info("wrote an image, start skiping ")).Times(1);
    EXPECT_CALL(*interface, notify(_, _)).Times(1);
    EXPECT_EQ(NotifyState::UPDATED, m_ptr->work(d));

//  main judge
    EXPECT_EQ(NotifyState::LOCKING, m_ptr->work(d));
}

/*
	data without any transfered object.
*/
TEST_F(DiscordNotifierTest, work_skipping) {
    DataResult d;
    //d.objs.push_back(obj_t{});
    d.names.push_back("person");
    EXPECT_EQ(NotifyState::SKIPPING, m_ptr->work(d));
}
