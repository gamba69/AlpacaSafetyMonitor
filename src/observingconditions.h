#pragma once

#include <AlpacaObservingConditions.h>
#include <Arduino.h>
#include <RunningAverage.h>
#include "config.h"
#include "meteo.h"
#include "version.h"

class ObservingConditions : public AlpacaObservingConditions {
  private:
    static uint8_t _n_observingconditionss;
    static ObservingConditions *_observingconditions_array[4];
    uint8_t _observingconditions_index;

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

    float temperature,
        humidity,
        pressure,
        rainrate,
        dewpoint,
        skytemp,
        noisedb,
        cloudcover,
        skyquality = 0,
        skybrightness = 0,
        windgust = 0,
        windspeed = 0,
        winddir = 0;
    RunningAverage temperature_ra = RunningAverage(1200),
                   humidity_ra = RunningAverage(1200),
                   pressure_ra = RunningAverage(1200),
                   rainrate_ra = RunningAverage(1200),
                   dewpoint_ra = RunningAverage(1200),
                   skytemp_ra = RunningAverage(1200),
                   noisedb_ra = RunningAverage(1200),
                   cloudcover_ra = RunningAverage(1200),
                   skyquality_ra = RunningAverage(1200),
                   skybrightness_ra = RunningAverage(1200),
                   windgust_ra = RunningAverage(1200),
                   windspeed_ra = RunningAverage(1200),
                   winddir_ra = RunningAverage(1200);
    unsigned long timelastupdate;
    const char *sensordescription = "Xiao Seeed ESP32S3/BMP280/AHT20/MLX90614";
    int _avgperiod = 30;
    int _refresh = 3;
    int _averaging = 1;
    bool _noise_as_fwhm = true;

    // immediate update
    std::function<void()> immediateUpdate = nullptr;

  public:
    ObservingConditions() : AlpacaObservingConditions() { _observingconditions_index = _n_observingconditionss++; }

    // Set current logger
    void setLogger(std::function<void(String)> logLineCallback = NULL, std::function<void(String)> logLinePartCallback = NULL, std::function<String()> logTimeCallback = NULL);

    bool begin();
    void update(Meteo meteo);

    // getters
    int getRefresh() { return _refresh; }
    int getAveragePeriod() { return _avgperiod; }

    // alpaca getters
    void aGetDescription(AsyncWebServerRequest *request) override;
    void aGetDriverInfo(AsyncWebServerRequest *request) override;
    void aGetDriverVersion(AsyncWebServerRequest *request) override;
    void aGetSensorDescription(AsyncWebServerRequest *request) { _alpacaServer->respond(request, sensordescription); }
    void aGetRainRate(AsyncWebServerRequest *request);
    void aGetTemperature(AsyncWebServerRequest *request);
    void aGetHumidity(AsyncWebServerRequest *request);
    void aGetDewPoint(AsyncWebServerRequest *request);
    void aGetPressure(AsyncWebServerRequest *request);
    void aGetSkyTemperature(AsyncWebServerRequest *request);
    void aGetCloudCover(AsyncWebServerRequest *request);
    void aGetStarFwhm(AsyncWebServerRequest *request);
    void aGetSkyBrightness(AsyncWebServerRequest *request);
    void aGetSkyQuality(AsyncWebServerRequest *request);
    void aGetWindDirection(AsyncWebServerRequest *request);
    void aGetWindGust(AsyncWebServerRequest *request);
    void aGetWindSpeed(AsyncWebServerRequest *request);

    void aGetAveragePeriod(AsyncWebServerRequest *request);
    void aGetTimeSinceLastUpdate(AsyncWebServerRequest *request);

    // alpaca setters
    void aPutAveragePeriod(AsyncWebServerRequest *request) {
        // ASCOM required hours
        float value;
        _alpacaServer->getParam(request, "averageperiod", value);
        _avgperiod = (int)round(3600. * value);
        _alpacaServer->respond(request, nullptr);
    }
    void aPutRefresh(AsyncWebServerRequest *request) {
        // _alpacaServer->getParam(request, "refresh", _refresh);
        if (immediateUpdate)
            immediateUpdate();
        _alpacaServer->respond(request, nullptr);
    }

    // alpaca json
    void aReadJson(JsonObject &root);
    void aWriteJson(JsonObject &root);

    // Set current logger
    void setImmediateUpdate(std::function<void()> immediateUpdateCallback = nullptr);
};
