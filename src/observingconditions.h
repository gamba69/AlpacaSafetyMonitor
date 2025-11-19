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
    float _avgperiod = 30;
    int _refresh = 3;

  public:
    ObservingConditions() : AlpacaObservingConditions() { _observingconditions_index = _n_observingconditionss++; }
    bool begin();
    void update(Meteo meteo);

    // getters
    int getRefresh() { return _refresh; }
    int getAveragePeriod() { return _avgperiod; }

    // alpaca getters
    void aGetDewPoint(AsyncWebServerRequest *request) { _alpacaServer->respond(request, dewpoint); }
    void aGetHumidity(AsyncWebServerRequest *request) { _alpacaServer->respond(request, humidity); }
    void aGetPressure(AsyncWebServerRequest *request) { _alpacaServer->respond(request, pressure); }
    void aGetSkyBrightness(AsyncWebServerRequest *request) { _alpacaServer->respond(request, skybrightness); }
    void aGetSkyTemperature(AsyncWebServerRequest *request) { _alpacaServer->respond(request, tempsky); }
    void aGetSkyQuality(AsyncWebServerRequest *request) { _alpacaServer->respond(request, skyquality); }
    void aGetStarFwhm(AsyncWebServerRequest *request) { _alpacaServer->respond(request, noisedb); }
    void aGetTemperature(AsyncWebServerRequest *request) { _alpacaServer->respond(request, temperature); }
    void aGetWindDirection(AsyncWebServerRequest *request) { _alpacaServer->respond(request, winddir); }
    void aGetWindGust(AsyncWebServerRequest *request) { _alpacaServer->respond(request, windgust); }
    void aGetWindSpeed(AsyncWebServerRequest *request) { _alpacaServer->respond(request, windspeed); }
    void aGetSensorDescription(AsyncWebServerRequest *request) { _alpacaServer->respond(request, sensordescription); }
    void aGetRainRate(AsyncWebServerRequest *request) { _alpacaServer->respond(request, rainrate); }
    void aGetCloudCover(AsyncWebServerRequest *request) { _alpacaServer->respond(request, cloudcover); }

    void aGetAveragePeriod(AsyncWebServerRequest *request);
    void aGetTimeSinceLastUpdate(AsyncWebServerRequest *request);

    // alpaca setters
    void aPutAveragePeriod(AsyncWebServerRequest *request) {
        // TODO ASCOM required hours unit!
        _alpacaServer->getParam(request, "averageperiod", _avgperiod);
        _alpacaServer->respond(request, nullptr);
    }
    void aPutRefresh(AsyncWebServerRequest *request) {
        _alpacaServer->getParam(request, "refresh", _refresh);
        _alpacaServer->respond(request, nullptr);
    }

    // alpaca json
    void aReadJson(JsonObject &root);
    void aWriteJson(JsonObject &root);
};
