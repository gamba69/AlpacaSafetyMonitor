#pragma once

#include <Arduino.h>
#include <String.h>

#define CAL_DATA_SIZE 32

#define CAL_BMP280_TEMPERATURE calData[CalDevice::BMP280Temperature]
#define CAL_BMP280_PRESSURE calData[CalDevice::BMP280Pressure]
#define CAL_AHT20_TEMPERATURE calData[CalDevice::AHT20Temperature]
#define CAL_AHT20_HUMIDITY calData[CalDevice::AHT20Humidity]
#define CAL_SHT45_TEMPERATURE calData[CalDevice::SHT45Temperature]
#define CAL_SHT45_HUMIDITY calData[CalDevice::SHT45Humidity]
#define CAL_DEW_POINT calData[CalDevice::DewPoint]
#define CAL_MLX90614_AMBIENT calData[CalDevice::MLX90614Ambient]
#define CAL_MLX90614_OBJECT calData[CalDevice::MLX90614Object]
#define CAL_MLX90614_SKYTEMP calData[CalDevice::MLX90614SkyTemperature]
#define CAL_MLX90614_CLOUDCOVER calData[CalDevice::MLX90614CloudCover]
#define CAL_TSL2591_SKYBRIGHTNESS calData[CalDevice::TSL2591SkyBrightness]
#define CAL_TSL2591_SKYQUALITY calData[CalDevice::TSL2591SkyQuality]
#define CAL_ANEMO4403_WINDSPEED calData[CalDevice::ANEMO4403WindSpeed]
#define CAL_ANEMO4403_WINDGUST calData[CalDevice::ANEMO4403WindGust]
#define CAL_UICPAL_RAINRATE calData[CalDevice::UICPALRainRate]
#define CAL_RG15_RAINRATE calData[CalDevice::RG15RainRate]

class CalDevice {
  public:
    static const int BMP280Temperature = 0;
    static const int BMP280Pressure = 1;
    static const int AHT20Temperature = 2;
    static const int AHT20Humidity = 3;
    static const int SHT45Temperature = 4;
    static const int SHT45Humidity = 5;
    static const int DewPoint = 6;
    static const int MLX90614Ambient = 7;
    static const int MLX90614Object = 8;
    static const int MLX90614SkyTemperature = 9;
    static const int MLX90614CloudCover = 10;
    static const int TSL2591SkyBrightness = 11;
    static const int TSL2591SkyQuality = 12;
    static const int ANEMO4403WindSpeed = 13;
    static const int ANEMO4403WindGust = 14;
    static const int UICPALRainRate = 15;
    static const int RG15RainRate = 16;
};

struct CalCoefficient {
    float a;
    float b;
    CalCoefficient() {
        a = 1;
        b = 0;
    }
    CalCoefficient(float _a, float _b) {
        a = _a;
        b = _b;
    }
};

extern CalCoefficient calData[CAL_DATA_SIZE];

void initCalPrefs();
void loadCalPrefs();
void saveCalPrefs();
float calibrate(float, CalCoefficient);
