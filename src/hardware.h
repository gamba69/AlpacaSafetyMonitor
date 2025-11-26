#pragma once

#include "main.h"
#include <Arduino.h>
#include <String.h>

#define HW_ENABLED_SIZE 64

extern bool hwEnabled[HW_ENABLED_SIZE];

enum Hardware {

    hwBmp280 = 0,
    hwAht20 = 1,
    hwMlx90614 = 2,
    hwTsl2591 = 3,
    hwUicpal = 4,
    hwAnemo4403 = 5,
    hwRg15 = 6,
    hwDs3231 = 7,

    alpacaOc = 10,
    alpacaSm = 11,

    ocRainRate = 20,
    ocTemperature = 21,
    ocHumidity = 22,
    ocDewPoint = 23,
    ocPressure = 24,
    ocSkyTemp = 25,
    ocCloudCover = 26,
    ocFwhm = 27,
    ocSkyQuality = 28,
    ocSkyBrightness = 29,
    ocWindDirection = 30,
    ocWindGust = 31,
    ocWindSpeed = 32,

    smRainRate = 40,
    smTemperature = 41,
    smHumidity = 42,
    smDewPoint = 43,
    smSkyTemp = 44,
    smWindSpeed = 45

};

void calcHwPrefs();
void initHwPrefs();
void loadHwPrefs();
void saveHwPrefs();
