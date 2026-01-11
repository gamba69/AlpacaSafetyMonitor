#pragma once

#include "config.h"
#include "meteoanm.h"
#include "meteotsl.h"
#include "meteosht.h"
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_TSL2591.h>
#include <Arduino.h>
#include <RG15.h>
#include <RunningAverage.h>
#include <Wire.h>

// Circular buffer functions
#define CB_SIZE 40
static float cb[CB_SIZE] = {0.};
static float cb_noise[CB_SIZE] = {0.};
static int cb_index = 0;
static float cb_avg = 0.0;
static float cb_rms = 0.0;

// Devices group bits
#define UICPAL_KICK (1UL << 0)
#define UICPAL_DONE (1UL << 1)
#define BMP280_KICK (1UL << 2)
#define BMP280_DONE (1UL << 3)
#define AHT20_KICK (1UL << 4)
#define AHT20_DONE (1UL << 5)
#define SHT45_KICK (1UL << 6)
#define SHT45_DONE (1UL << 7)
#define MLX90614_KICK (1UL << 8)
#define MLX90614_DONE (1UL << 9)
#define TSL2591_KICK (1UL << 10)
#define TSL2591_DONE (1UL << 11)
#define ANEMO4403_KICK (1UL << 12)
#define ANEMO4403_DONE (1UL << 13)
#define RG15_KICK (1UL << 14)
#define RG15_DONE (1UL << 15)

#ifndef METEO_H
#define METEO_H

class Meteo {
  public:
    Meteo() = default;
    Meteo(Meteo &&) = delete;
    Meteo(const Meteo &) = delete;
    // attributes
    volatile struct {
        float uicpal_rate, rg15_rate, rain_rate;
        float bmp_temperature, bmp_pressure;
        float aht_temperature, aht_humidity;
        float sht_temperature, sht_humidity;
        float temperature, humidity, dew_point;
        float mlx_tempamb, mlx_tempobj, sky_temperature, cloud_cover;
        float noise_db;
        float sky_quality, sky_brightness;
        float wind_direction, wind_speed, wind_gust;
    } sensors = {0};
    // methods
    void update(bool force = false);
    //  setters
    //  getters
    // const std::string &getName() const;
    void begin();
    // Set current logger
    void setLogger(const int source, std::function<void(String, const int)> logLineCallback = nullptr, std::function<void(String, const int)> logLinePartCallback = nullptr, std::function<String()> logTimeCallback = nullptr);

  private:
    // Formatting
    String trimmed(float, int);
    // Wind gust calculation
    RunningAverage wind_gust_ra = RunningAverage(40);
    // Last log message
    unsigned long last_message = 0;
    // Logger println
    std::function<void(String, const int)> logLine = nullptr;
    // Logger print
    std::function<void(String, const int)> logLinePart = nullptr;
    // Logger time function
    std::function<String()> logTime = nullptr;
    // Logger source
    int logSource;
    // Print a log message, can be overwritten
    virtual void logMessage(String msg, bool showtime = true);
    // Print a part of log message, can be overwritten
    virtual void logMessagePart(String msg, bool showtime = false);

    // CB functions
    float tsky_calc(float ts, float ta);
    float cb_avg_calc();
    float cb_rms_calc();
    void cb_add(float value);
    float cb_noise_db_calc();
    float cb_snr_calc();

    TSL2591Settings autoGainSettings[TSL_SETTINGS_SIZE];
    void beginTslAutoGain(Adafruit_TSL2591 *);
    void setTslAutoGain(Adafruit_TSL2591 *, int);
    TSL2591Data getTslAutoGain(Adafruit_TSL2591 *);
    float calcLuxAutoGain(TSL2591Data);
    float calcSqmAutoGain(TSL2591Data);

    EventGroupHandle_t xDevicesGroup;

    // UICPAL Task
    TaskHandle_t updateUicpalHandle = NULL;
    static void updateUicpalWrapper(void *parameter) {
        // Cast parameter back to the class instance pointer
        Meteo *instance = static_cast<Meteo *>(parameter);
        // Call the actual member function
        instance->updateUicpal();
    }
    void updateUicpal(void);

    // RG15 Task
    TaskHandle_t updateRg15Handle = NULL;
    static void updateRg15Wrapper(void *parameter) {
        // Cast parameter back to the class instance pointer
        Meteo *instance = static_cast<Meteo *>(parameter);
        // Call the actual member function
        instance->updateRg15();
    }
    void updateRg15(void);

    // BMP280 Task
    TaskHandle_t updateBmp280Handle = NULL;
    static void updateBmp280Wrapper(void *parameter) {
        // Cast parameter back to the class instance pointer
        Meteo *instance = static_cast<Meteo *>(parameter);
        // Call the actual member function
        instance->updateBmp280();
    }
    void updateBmp280(void);

    // AHT20 Task
    TaskHandle_t updateAht20Handle = NULL;
    static void updateAht20Wrapper(void *parameter) {
        // Cast parameter back to the class instance pointer
        Meteo *instance = static_cast<Meteo *>(parameter);
        // Call the actual member function
        instance->updateAht20();
    }
    void updateAht20(void);

    // SHT45 Task
    TaskHandle_t updateSht45Handle = NULL;
    static void updateSht45Wrapper(void *parameter) {
        // Cast parameter back to the class instance pointer
        Meteo *instance = static_cast<Meteo *>(parameter);
        // Call the actual member function
        instance->updateSht45();
    }
    void updateSht45(void);

    // MLX90614 Task
    TaskHandle_t updateMlx90614Handle = NULL;
    static void updateMlx90614Wrapper(void *parameter) {
        // Cast parameter back to the class instance pointer
        Meteo *instance = static_cast<Meteo *>(parameter);
        // Call the actual member function
        instance->updateMlx90614();
    }
    void updateMlx90614(void);

    // TSL2591 Task
    TaskHandle_t updateTsl2591Handle = NULL;
    static void updateTsl2591Wrapper(void *parameter) {
        // Cast parameter back to the class instance pointer
        Meteo *instance = static_cast<Meteo *>(parameter);
        // Call the actual member function
        instance->updateTsl2591();
    }
    void updateTsl2591(void);

    // ANEMO4403 Wind Speed Task
    TaskHandle_t updateAnemo4403SpeedHandle = NULL;
    static void updateAnemo4403SpeedWrapper(void *parameter) {
        // Cast parameter back to the class instance pointer
        Meteo *instance = static_cast<Meteo *>(parameter);
        // Call the actual member function
        instance->updateAnemo4403Speed();
    }
    void updateAnemo4403Speed(void);

    // ANEMO4403 Wind Gust Task
    TaskHandle_t updateAnemo4403GustHandle = NULL;
    static void updateAnemo4403GustWrapper(void *parameter) {
        // Cast parameter back to the class instance pointer
        Meteo *instance = static_cast<Meteo *>(parameter);
        // Call the actual member function
        instance->updateAnemo4403Gust();
    }
    void updateAnemo4403Gust(void);
};

#endif