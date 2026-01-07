#pragma once

#include "main.h"
#include <Arduino.h>
#include <String.h>

#define HARDWARE_BMP280 hwEnabled[hwBmp280]
#define HARDWARE_AHT20 hwEnabled[hwAht20]
#define HARDWARE_SHT45 hwEnabled[hwSht45]
#define HARDWARE_MLX90614 hwEnabled[hwMlx90614]
#define HARDWARE_TSL2591 hwEnabled[hwTsl2591]
#define HARDWARE_UICPAL hwEnabled[hwUicpal]
#define HARDWARE_ANEMO4403 hwEnabled[hwAnemo4403]
#define HARDWARE_RG15 hwEnabled[hwRg15]
#define HARDWARE_DS3231 hwEnabled[hwDs3231]

#define ALPACA_OBSCON hwEnabled[alpacaObscon]
#define ALPACA_SAFEMON hwEnabled[alpacaSafemon]

#define OBSCON_RAINRATE hwEnabled[obsconRainRate]
#define OBSCON_TEMPERATURE hwEnabled[obsconTemperature]
#define OBSCON_HUMIDITY hwEnabled[obsconHumidity]
#define OBSCON_DEWPOINT hwEnabled[obsconDewPoint]
#define OBSCON_PRESSURE hwEnabled[obsconPressure]
#define OBSCON_SKYTEMP hwEnabled[obsconSkyTemp]
#define OBSCON_CLOUDCOVER hwEnabled[obsconCloudCover]
#define OBSCON_FWHM hwEnabled[obsconFwhm]
#define OBSCON_SKYQUALITY hwEnabled[obsconSkyQuality]
#define OBSCON_SKYBRIGHTNESS hwEnabled[obsconSkyBrightness]
#define OBSCON_WINDDIR hwEnabled[obsconWindDirection]
#define OBSCON_WINDGUST hwEnabled[obsconWindGust]
#define OBSCON_WINDSPEED hwEnabled[obsconWindSpeed]

#define SAFEMON_RAINRATE hwEnabled[safemonRainRate]
#define SAFEMON_TEMPERATURE hwEnabled[safemonTemperature]
#define SAFEMON_HUMIDITY hwEnabled[safemonHumidity]
#define SAFEMON_DEWPOINT hwEnabled[safemonDewPoint]
#define SAFEMON_SKYTEMP hwEnabled[safemonSkyTemp]
#define SAFEMON_WINDSPEED hwEnabled[safemonWindSpeed]

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
    hwSht45 = 8,

    alpacaObscon = 10,
    alpacaSafemon = 11,

    obsconRainRate = 20,
    obsconTemperature = 21,
    obsconHumidity = 22,
    obsconDewPoint = 23,
    obsconPressure = 24,
    obsconSkyTemp = 25,
    obsconCloudCover = 26,
    obsconFwhm = 27,
    obsconSkyQuality = 28,
    obsconSkyBrightness = 29,
    obsconWindDirection = 30,
    obsconWindGust = 31,
    obsconWindSpeed = 32,

    safemonRainRate = 40,
    safemonTemperature = 41,
    safemonHumidity = 42,
    safemonDewPoint = 43,
    safemonSkyTemp = 44,
    safemonWindSpeed = 45
};

void calcHwPrefs();
void initHwPrefs();
void loadHwPrefs();
void saveHwPrefs();
