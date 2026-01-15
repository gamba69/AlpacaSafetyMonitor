#pragma once
#include "config.h"
#include <Adafruit_TSL2591.h>

#define TSL_SETTINGS_SIZE 7
#define TSL_INTERRUPT_LOWER_PERCENT 8.798916064 // -8.798916064%
#define TSL_INTERRUPT_UPPER_PERCENT 9.647819614 // +9.647819614%
#define TSL_KICK (1UL << 0)

class TSL2591Events {
  public:
    static const int THRESHOLD_INTERRUPT = (1 << 0);
    static const int DATAREADY_CALLBACK = (1 << 1);
    static const int THRESHOLD_CALLBACK = (1 << 1);
};

class TSL2591Settings {
  public:
    tsl2591Gain_t gain;
    tsl2591IntegrationTime_t time;
    uint low;
    uint high;
    TSL2591Settings() : gain(TSL2591_GAIN_LOW), time(TSL2591_INTEGRATIONTIME_100MS), low(0), high(UINT_MAX) {}
    TSL2591Settings(tsl2591Gain_t g, tsl2591IntegrationTime_t t, uint l, uint h) : gain(g), time(t), low(l), high(h) {}
};

class TSL2591Data {
  public:
    tsl2591Gain_t gain;
    tsl2591IntegrationTime_t time;
    uint32_t luminosity;
    TSL2591Data() {};
    TSL2591Data(tsl2591Gain_t g, tsl2591IntegrationTime_t t, uint32_t l) : gain(g), time(t), luminosity(l) {}
};

class TSL2591AutoGain {
  private:
    Adafruit_TSL2591 tsl;
    TaskHandle_t task;
    int events = 0;
    TSL2591Data lastData;
    SemaphoreHandle_t dataMutex;

    TSL2591Settings settings[TSL_SETTINGS_SIZE];
    int currentIndex;

    // EventGroupHandle_t xExtEvents = nullptr;
    // unsigned long xExtBit = 0;

    static void taskWrapper(void *p);
    void updatingTask();
    EventGroupHandle_t xTslEvents;
    TSL2591Data getLastData();

    float timeAsMillis(tsl2591IntegrationTime_t);
    float gainAsMulti(tsl2591Gain_t);
    void setAutoGain(int);
    void setThresholds(uint16_t);

    std::function<void(String, const int)> logLine = nullptr;
    std::function<void(String, const int)> logLinePart = nullptr;
    std::function<String()> logTime = nullptr;
    int logSource;

    std::function<void()> dataReadyCallback = nullptr;

    virtual void logMessage(String msg, bool showtime = true);
    virtual void logMessagePart(String msg, bool showtime = false);
    String gainAsString(tsl2591Gain_t);
    String timeAsString(tsl2591IntegrationTime_t);

  public:
    TSL2591AutoGain() : tsl(2591), currentIndex(3) {}
    bool begin(int = 0);
    TSL2591Data getData();
    void forceUpdate();
    float calculateLux(const TSL2591Data &);
    float calculateSQM(const TSL2591Data &);
    void setLogger(const int source, std::function<void(String, const int)> logLineCallback = nullptr, std::function<void(String, const int)> logLinePartCallback = nullptr, std::function<String()> logTimeCallback = nullptr);
    void setDataReadyCallback(std::function<void()> dataReadyCallback = nullptr);
};