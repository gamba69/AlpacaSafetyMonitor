#pragma once
#include "AlpacaObservingConditions.h"
#include "RunningAverage.h"
#include "config.h"
#include "meteo.h"
#include <Arduino.h>

class ObservingConditions : public AlpacaObservingConditions {
  private:
    static uint8_t _n_observingconditionss;
    static ObservingConditions *_observingconditions_array[4];
    uint8_t _observingconditions_index;
    float temperature,
        humidity,
        pressure,
        rainrate,
        dewpoint,
        tempsky,
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
                   tempsky_ra = RunningAverage(1200),
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

    // immediate update
    std::function<void()> immediateUpdate = nullptr;

  public:
    ObservingConditions() : AlpacaObservingConditions() { _observingconditions_index = _n_observingconditionss++; }
    bool begin();
    void update(Meteo meteo);

    // getters
    int getRefresh() { return _refresh; }
    int getAveragePeriod() { return _avgperiod; }

    // alpaca getters
    void aGetRainRate(AsyncWebServerRequest *request);
    void aGetTemperature(AsyncWebServerRequest *request);
    void aGetHumidity(AsyncWebServerRequest *request);
    void aGetDewPoint(AsyncWebServerRequest *request);
    void aGetPressure(AsyncWebServerRequest *request);
    void aGetSkyTemperature(AsyncWebServerRequest *request);
    void aGetCloudCover(AsyncWebServerRequest *request);
    void aGetStarFwhm(AsyncWebServerRequest *request);
    void aGetSkyBrightness(AsyncWebServerRequest *request) { _alpacaServer->respond(request, skybrightness); }
    void aGetSkyQuality(AsyncWebServerRequest *request) { _alpacaServer->respond(request, skyquality); }
    void aGetWindDirection(AsyncWebServerRequest *request) { _alpacaServer->respond(request, winddir); }
    void aGetWindGust(AsyncWebServerRequest *request) { _alpacaServer->respond(request, windgust); }
    void aGetWindSpeed(AsyncWebServerRequest *request) { _alpacaServer->respond(request, windspeed); }
    void aGetSensorDescription(AsyncWebServerRequest *request) { _alpacaServer->respond(request, sensordescription); }

    void aGetAveragePeriod(AsyncWebServerRequest *request);
    void aGetTimeSinceLastUpdate(AsyncWebServerRequest *request);

    // alpaca setters
    void aPutAveragePeriod(AsyncWebServerRequest *request) {
        // TODO ASCOM required hours unit!
        _alpacaServer->getParam(request, "averageperiod", _avgperiod);
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
