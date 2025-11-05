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

    float limit_tamb = 0.;   // freezing below this
    float limit_tsky = -15.; //-15.;   // cloudy above this
    float limit_humid = 90.; // risk for electronics above this
    float limit_dew = 5.;    // risk for optics with temp - dewpoint below this

    bool temp_prove = true;
    float temp_low_limit = -15;
    float temp_high_limit = -13;
    bool skytemp_prove = true;
    float skytemp_low_limit = -18;
    float skytemp_high_limit = -16;
    bool humi_prove = true;
    float humi_low_limit = 90;
    float humi_high_limit = 95;
    bool dewdelta_prove = true;
    float dewdelta_low_limit = 4;
    float dewdelta_high_limit = 5;
    bool rain_prove = true;
    float rain_off_delay = 600;
    bool wind_prove = true;
    // WIND

    float delay2open = 1200.; // waiting time before open roof after a safety close
    float delay2close = 120.; // waiting time before close roof with continuos overall safety waring for this
    float time2open, time2close = 0;

    bool status_tamb, status_tsky, status_humid, status_dew, status_weather, instant_status, status_roof = false;

    bool _issafe = false; // overall meteo safety status

    // acquired parameters
    float temperature, humidity, dewpoint, dewpoint_delta, tempsky, rainrate;

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