#pragma once
#include "yaml-cpp/yaml.h"
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <exception>
#include <cassert>

#ifndef DEFAULT_CFG_PATH 
#define DEFAULT_CFG_PATH "/etc/iot-config.yaml"
#endif


class Config {
 public:
    virtual time_t getTimeForcus() = 0;
    virtual time_t getTimeSkippingDectection() = 0;
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
private:
    const std::string m_file;
    YAML::Node m_config;
    explicit YamlConfig(const std::string path) : m_file(path), Config()
    {
        m_config = YAML::LoadFile(m_file);
    }

public:
    static Config& getInstance();

    time_t getTimeForcus() override;
    time_t getTimeSkippingDectection() override;
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
    YAML::Node operator[](std::string k) { return m_config[k]; }
    void sync()
    {
        std::ofstream fout(DEFAULT_CFG_PATH);
        fout << m_config;
    }
};

class JSONConfig : public Config
{
private:
    const std::string m_file;

public:
    static Config& getInstance();

    time_t getTimeForcus() override;
    time_t getTimeSkippingDectection() override;
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
    //YAML::Node operator[](std::string k) { return m_config[k]; }
    void sync()
    {
        std::ofstream fout(DEFAULT_CFG_PATH);
    //    /fout << m_config;
    }
};

void testConfig(Config&);
