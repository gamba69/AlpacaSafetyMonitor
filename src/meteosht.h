#pragma once

#include "SHT4x.h"
#include <Arduino.h>
#include <Wire.h>

struct SHT45Data {
    float temperature;
    float humidity;
    bool valid;
};

struct HeatingParams {
    float humMin, humMax;
    uint32_t intervalMin, intervalMax;
    uint8_t cmd;
    uint32_t cooldown;
    uint8_t cycles;
};

class SHT45AutoHeat {
  public:
    SHT45AutoHeat();
    ~SHT45AutoHeat();

    bool begin();
    SHT45Data readData();

  private:
    SHT4x sht;
    TaskHandle_t task;
    SemaphoreHandle_t semaphore;
    volatile float humidity;
    uint32_t lastHeat;
    uint32_t nextAllowed;

    static const HeatingParams table[5];

    static void taskWrapper(void *p);
    void heatingTask();
    void doHeat(uint8_t cmd);
    void updateHumidity();
    const HeatingParams *getParams(float h);
    uint32_t getHeatDuration(uint8_t cmd);
};
