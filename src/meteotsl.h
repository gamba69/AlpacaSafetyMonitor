#pragma once
#include "config.h"
#include <Adafruit_TSL2591.h>

#define TSL_SETTINGS_SIZE 7

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
    TSL2591Data(tsl2591Gain_t g, tsl2591IntegrationTime_t t, uint32_t l) : gain(g), time(t), luminosity(l) {}
};

class TSL2591AutoGain {
  private:
    Adafruit_TSL2591 tsl;
    TSL2591Settings settings[TSL_SETTINGS_SIZE];
    int currentIndex;
    float timeAsMillis(tsl2591IntegrationTime_t t);
    float gainAsMulti(tsl2591Gain_t g);
    void setAutoGain(int index);

  public:
    TSL2591AutoGain();
    bool begin();
    TSL2591Data getData();
    float calculateLux(const TSL2591Data &data);
    float calculateSQM(const TSL2591Data &data);
};