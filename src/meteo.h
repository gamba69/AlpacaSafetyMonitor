#pragma once

#include "config.h"
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_MLX90614.h>
#include <Arduino.h>
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

#ifndef METEO_H
#define METEO_H

class Meteo {
  public:
    // attributes
    volatile float
        rain_rate,
        bmp_temperature,
        bmp_pressure,
        aht_temperature,
        aht_humidity,
        mlx_tempamb,
        mlx_tempobj,
        sky_temperature,
        noise_db,
        amb_temperature,
        dew_point,
        cloud_cover,
        sky_quality,
        sky_brightness,
        wind_direction,
        wind_speed,
        wind_gust;
    // methods
    void update(bool force = false);
    //  setters
    //  getters
    // const std::string &getName() const;
    void begin();
    // Set current logger
    void setLogger(std::function<void(String)> logLineCallback = nullptr, std::function<void(String)> logLinePartCallback = nullptr, std::function<String()> logTimeCallback = nullptr);

  private:
    // Last log message
    unsigned long last_message = 0;
    // Logger println
    std::function<void(String)> logLine = nullptr;
    // Logger print
    std::function<void(String)> logLinePart = nullptr;
    // Logger time function
    std::function<String()> logTime = nullptr;
    // Print a log message, can be overwritten
    virtual void logMessage(String msg, bool showtime = true);
    // Print a part of log message, can be overwritten
    virtual void logMessagePart(String msg, bool showtime = false);

    EventGroupHandle_t xDevicesGroup;

    TaskHandle_t updateTsl2591Handle = NULL;
    static void updateTsl2591Wrapper(void *parameter) {
        // Cast parameter back to the class instance pointer
        Meteo *instance = static_cast<Meteo *>(parameter);
        // Call the actual member function
        instance->updateTsl2591();
    }
    void updateTsl2591(void);

    TaskHandle_t updateUicpalHandle = NULL;
    static void updateUicpalWrapper(void *parameter) {
        // Cast parameter back to the class instance pointer
        Meteo *instance = static_cast<Meteo *>(parameter);
        // Call the actual member function
        instance->updateUicpal();
    }
    void updateUicpal(void);
};

#endif