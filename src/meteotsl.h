#pragma once

#include <Adafruit_TSL2591.h>

#define TSL_SETTINGS_SIZE 7

class TslSetting {
  public:
    tsl2591Gain_t gain;
    tsl2591IntegrationTime_t time;
    uint low;
    uint high;
    TslSetting()
        : gain(TSL2591_GAIN_LOW), time(TSL2591_INTEGRATIONTIME_100MS),
          low(0), high(UINT_MAX) {}
    TslSetting(tsl2591Gain_t gain, tsl2591IntegrationTime_t time, uint low, uint high)
        : gain(gain), time(time), low(low), high(high) {}
};

class TslAutoLum {
  public:
    tsl2591Gain_t gain;
    tsl2591IntegrationTime_t time;
    uint32_t luminosity;
    TslAutoLum(tsl2591Gain_t gain, tsl2591IntegrationTime_t time, uint32_t luminosity)
        : gain(gain), time(time), luminosity(luminosity) {}
};