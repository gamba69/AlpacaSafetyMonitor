#pragma once
#include "config.h"
#include <Arduino.h>
// I2C Sensors
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_MLX90614.h>
#include <Wire.h>

// Circular buffer functions
#define CB_SIZE 24
static float cb[CB_SIZE] = {0.};
static float cb_noise[CB_SIZE] = {0.};
static int cb_index = 0;
static float cb_avg = 0.0;
static float cb_rms = 0.0;

#ifndef METEO_H
#define METEO_H

class Meteo {
  public:
    // attributes
    std::string Name;
    float
        bmp_temperature,
        bmp_pressure,
        aht_temperature,
        aht_humidity,
        mlx_tempamb,
        mlx_tempobj,
        tempsky,
        noise_db,
        dewpoint,
        cloudcover,
        skyquality,
        skybrightness;
    // sensors
    // methods
    void update(unsigned long measureDelay);
    Meteo(const std::string &newName); // constructor place
    // setters
    // getters
    const std::string &getName() const;
    void begin();
    // Set current logger
    void setLogger(Print *print, std::function<String()> logtime = NULL);

  private:
    // Logger print
    Print *logger = &Serial;
    // Logger time function
    std::function<String()> logtime = NULL;
    // Print a log message, can be overwritten
    virtual void logMessage(String msg, bool showtime = true);
    // Print a part of log message, can be overwritten
    virtual void logMessagePart(String msg, bool showtime = false);
};

#endif