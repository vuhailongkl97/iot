#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <exception>
#include <cassert>
#include <memory>

class Config
{
public:
    bool update(std::string cfg)
    {
        auto ret = parse(cfg);
        sync();
        return ret;
    }
    virtual bool status() = 0;
    virtual bool parse(std::string) = 0;
    virtual time_t getTimeForcus() = 0;
    virtual time_t getTimeSkippingDetection() = 0;
    virtual int getMinQueueEntryLimit() = 0;
    virtual std::string getNotifyAPI() = 0;
    virtual int getDelay4Cap() = 0;
    virtual int getHTTPPort() = 0;
    virtual std::string getNamesFile() = 0;
    virtual std::string getCfgFile() = 0;
    virtual std::string getWeightFile() = 0;
    virtual float getThreshold() = 0;
    virtual void setThreshold(float) = 0;
    virtual std::string getSrc() = 0;
    virtual float getCorrectRate() = 0;
    virtual std::string getBoardName() = 0;
    virtual void show() = 0;
    virtual void sync() = 0;
};

class YamlConfig : public Config
{
public:
    struct impl;

private:
    const std::string m_file;
    std::unique_ptr<impl> m_config;
    explicit YamlConfig(const std::string path, impl* p);
    impl operator[](std::string k);

public:
    static Config& getInstance(const char*);

    bool status() override;
    bool parse(std::string cfg) override;
    time_t getTimeForcus() override;
    time_t getTimeSkippingDetection() override;
    int getMinQueueEntryLimit() override;
    std::string getNotifyAPI() override;
    int getDelay4Cap() override;
    int getHTTPPort() override;
    std::string getNamesFile() override;
    std::string getCfgFile() override;
    std::string getWeightFile() override;
    float getThreshold() override;
    void setThreshold(float) override;
    std::string getSrc() override;
    float getCorrectRate() override;
    std::string getBoardName() override;
    void show() override;
    void sync() override;
};

class JSONConfig : public Config
{
public:
    struct impl;

private:
    const std::string m_file;
    std::unique_ptr<impl> m_config;
    explicit JSONConfig(const std::string path, impl* p);
    impl operator[](std::string k);

public:
    static Config& getInstance(const char*);

    bool status() override;
    bool parse(std::string cfg) override;
    time_t getTimeForcus() override;
    time_t getTimeSkippingDetection() override;
    int getMinQueueEntryLimit() override;
    std::string getNotifyAPI() override;
    int getDelay4Cap() override;
    int getHTTPPort() override;
    std::string getNamesFile() override;
    std::string getCfgFile() override;
    std::string getWeightFile() override;
    float getThreshold() override;
    void setThreshold(float) override;
    std::string getSrc() override;
    float getCorrectRate() override;
    std::string getBoardName() override;
    void show() override;
    void sync() override;
};

void testConfig(Config&);
