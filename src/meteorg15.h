#pragma once

#include "config.h"
#include <RG15.h>

#define RG_KICK (1UL << 0)

class RGEvents {
  public:
    static const int DATAREADY_CALLBACK = (1 << 0);
};

struct RGData {
    float rainfallIntensity = 0;
};

class RGAsync {
  private:
    RG15 rg15;
    TaskHandle_t task;
    int events = 0;
    RGData lastData;
    SemaphoreHandle_t dataMutex;

    static void taskWrapper(void *p);
    void updatingTask();
    EventGroupHandle_t xRg15Events;
    RGData getLastData();

    std::function<void(String, const int)> logLine = nullptr;
    std::function<void(String, const int)> logLinePart = nullptr;
    std::function<String()> logTime = nullptr;
    int logSource;

    std::function<void()> dataReadyCallback = nullptr;

    virtual void logMessage(String msg, bool showtime = true);
    virtual void logMessagePart(String msg, bool showtime = false);

  public:
    RGAsync() : rg15(Serial0) {}
    bool begin(int = 0);
    RGData getData();
    void forceUpdate();
    void setLogger(const int source, std::function<void(String, const int)> logLineCallback = nullptr, std::function<void(String, const int)> logLinePartCallback = nullptr, std::function<String()> logTimeCallback = nullptr);
    void setDataReadyCallback(std::function<void()> dataReadyCallback = nullptr);
};