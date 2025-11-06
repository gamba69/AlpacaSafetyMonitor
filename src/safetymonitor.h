#pragma once
#include "AlpacaSafetyMonitor.h"
#include "config.h"
#include "meteo.h"
#include <Arduino.h>

class SafetyMonitor : public AlpacaSafetyMonitor {
  private:
    // Logger println
    std::function<void(String)> logLine = NULL;
    // Logger print
    std::function<void(String)> logLinePart = NULL;
    // Logger time function
    std::function<String()> logTime = NULL;
    // Print a log message, can be overwritten
    virtual void logMessage(String msg, bool showtime = true);
    // Print a part of log message, can be overwritten
    virtual void logMessagePart(String msg, bool showtime = false);

    static uint8_t _n_safetymonitors;
    static SafetyMonitor *_safetymonitor_array[4];
    uint8_t _safetymonitor_index;

    // Rain
    bool rain_prove = true;
    int rain_cessation_delay = 600;
    bool rain_safe = false;
    // Temp
    bool temp_prove = true;
    float temp_lower_limit = -15.;
    float temp_upper_limit = -13.;
    bool temp_safe = false;
    // Humi
    bool humi_prove = true;
    float humi_lower_limit = 90.;
    float humi_upper_limit = 95.;
    bool humi_safe = false;
    // Dew Delta
    bool dewdelta_prove = true;
    float dewdelta_lower_limit = 4.;
    float dewdelta_upper_limit = 5.;
    bool dewdelta_safe = false;
    // Sky Temp
    bool skytemp_prove = true;
    float skytemp_lower_limit = -18.;
    float skytemp_upper_limit = -16.;
    bool skytemp_safe = false;
    // Wind
    bool wind_prove = true;
    float wind_lower_limit = 6;
    float wind_upper_limit = 7;
    bool wind_safe = false;

    bool _issafe = false; // overall meteo safety status

    // acquired parameters
    float temperature, humidity, dewpoint, dewpoint_delta, tempsky, rainrate, windspeed;

  public:
    SafetyMonitor() : AlpacaSafetyMonitor() { _safetymonitor_index = _n_safetymonitors++; }

    // Set current logger
    void setLogger(std::function<void(String)> logLineCallback = NULL, std::function<void(String)> logLinePartCallback = NULL, std::function<String()> logTimeCallback = NULL);

    bool begin();
    void update(Meteo meteo, unsigned long measureDelay);

    // alpaca getters
    void aGetIsSafe(AsyncWebServerRequest *request) { _alpacaServer->respond(request, _issafe); }

    // alpaca setters

    // alpaca json
    void aReadJson(JsonObject &root);
    void aWriteJson(JsonObject &root);
};