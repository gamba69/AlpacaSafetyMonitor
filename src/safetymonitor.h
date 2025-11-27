#pragma once

#include "config.h"
#include "meteo.h"
#include "version.h"
#include <AlpacaSafetyMonitor.h>
#include <Arduino.h>

enum RainRateState {
    WET,
    AWAIT_WET,
    DRY,
    AWAIT_DRY
};

enum SafeUnsafeStatus {
    SAFE,
    AWAIT_SAFE,
    UNSAFE,
    AWAIT_UNSAFE
};

class SafetyMonitor : public AlpacaSafetyMonitor {
  private:
    static uint8_t _n_safetymonitors;
    static SafetyMonitor *_safetymonitor_array[4];
    uint8_t _safetymonitor_index;

    // Last log message
    unsigned long last_message = 0;
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

    // Rain
    bool rain_prove = true;
    int rainrate_wet_delay = 0;
    int rainrate_dry_delay = 60;
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

    bool is_safe = false; // overall meteo safety status
    int safe_delay = 600;
    int unsafe_delay = 0;

    // acquired parameters
    float rainrate, temperature, humidity, dewpoint, dewpoint_delta, skytemp, windspeed;

    // Rainrate countdown logic
    bool rain_init = false;
    float rainrate_curr, rainrate_prev;
    unsigned long rainrate_occur = 0;
    RainRateState rainrate_state;

    // Safe countdown logic
    bool safeunsafe_init = false;
    float safeunsafe_curr, safeunsafe_prev;
    unsigned long safeunsafe_occur = 0;
    SafeUnsafeStatus safeunsafe_state;

  public:
    SafetyMonitor() : AlpacaSafetyMonitor() { _safetymonitor_index = _n_safetymonitors++; }

    // Set current logger
    void setLogger(std::function<void(String)> logLineCallback = NULL, std::function<void(String)> logLinePartCallback = NULL, std::function<String()> logTimeCallback = NULL);

    int getRainRateCountdown();
    int getSafeUnsafeCountdown();

    bool begin();
    void update(Meteo meteo);

    // alpaca getters
    void aGetDescription(AsyncWebServerRequest *request) override;
    void aGetDriverInfo(AsyncWebServerRequest *request) override;
    void aGetDriverVersion(AsyncWebServerRequest *request) override;
    void aGetIsSafe(AsyncWebServerRequest *request) { _alpacaServer->respond(request, is_safe); }

    // alpaca setters

    // alpaca json
    void aReadJson(JsonObject &root);
    void aWriteJson(JsonObject &root);
};