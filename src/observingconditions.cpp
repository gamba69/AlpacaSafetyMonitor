#include "observingconditions.h"
#include "meteo.h"

// cannot call member functions directly from interrupt, so need these helpers for up to 1 ObservingConditions
uint8_t ObservingConditions::_n_observingconditionss = 0;
ObservingConditions *ObservingConditions::_observingconditions_array[4] = {nullptr, nullptr, nullptr, nullptr};

void ObservingConditions::setImmediateUpdate(std::function<void()> immediateUpdateCallcback) {
    immediateUpdate = immediateUpdateCallcback;
}

bool ObservingConditions::begin() {
    _observingconditions_array[_observingconditions_index] = this;
    return true;
}

void ObservingConditions::update(Meteo meteo) {
    temperature = (meteo.bmp_temperature + meteo.aht_temperature) / 2;
    humidity = meteo.aht_humidity;
    pressure = meteo.bmp_pressure;
    rainrate = meteo.rainrate;
    dewpoint = meteo.dewpoint;
    tempsky = meteo.tempsky;
    noisedb = meteo.noise_db;
    cloudcover = meteo.cloudcover;
    skyquality = meteo.skyquality;
    skybrightness = meteo.skybrightness;
    temperature_ra.add(temperature);
    humidity_ra.add(humidity);
    pressure_ra.add(pressure);
    rainrate_ra.add(rainrate);
    dewpoint_ra.add(dewpoint);
    tempsky_ra.add(tempsky);
    noisedb_ra.add(noisedb);
    cloudcover_ra.add(cloudcover);
    skyquality_ra.add(skyquality);
    skybrightness_ra.add(skybrightness);
    timelastupdate = millis();
};

void ObservingConditions::aGetTimeSinceLastUpdate(AsyncWebServerRequest *request) {
    float seconds = (millis() - timelastupdate) / 1000;
    _alpacaServer->respond(request, seconds);
}

void ObservingConditions::aGetAveragePeriod(AsyncWebServerRequest *request) {
    // TODO ASCOM required hours unit!
    _alpacaServer->respond(request, _avgperiod);
}

void ObservingConditions::aGetRainRate(AsyncWebServerRequest *request) {
    // averaged
    int averaging = _avgperiod / _refresh;
    if (averaging == 0)
        averaging = 1;
    float value = rainrate_ra.getAverageLast(
        averaging > rainrate_ra.getCount() ? rainrate_ra.getCount() : averaging);
    value = round(10. * value) / 10.;
    _alpacaServer->respond(request, value);
}

void ObservingConditions::aGetTemperature(AsyncWebServerRequest *request) {
    // averaged
    int averaging = _avgperiod / _refresh;
    if (averaging == 0)
        averaging = 1;
    float value = temperature_ra.getAverageLast(
        averaging > temperature_ra.getCount() ? temperature_ra.getCount() : averaging);
    value = round(10. * value) / 10.;
    _alpacaServer->respond(request, value);
}

void ObservingConditions::aGetHumidity(AsyncWebServerRequest *request) {
    // averaged
    int averaging = _avgperiod / _refresh;
    if (averaging == 0)
        averaging = 1;
    float value = humidity_ra.getAverageLast(
        averaging > humidity_ra.getCount() ? humidity_ra.getCount() : averaging);
    value = round(1. * value) / 1.;
    _alpacaServer->respond(request, value);
}

void ObservingConditions::aGetDewPoint(AsyncWebServerRequest *request) {
    // averaged
    int averaging = _avgperiod / _refresh;
    if (averaging == 0)
        averaging = 1;
    float value = dewpoint_ra.getAverageLast(
        averaging > dewpoint_ra.getCount() ? dewpoint_ra.getCount() : averaging);
    value = round(10. * value) / 10.;
    _alpacaServer->respond(request, value);
}

void ObservingConditions::aGetPressure(AsyncWebServerRequest *request) {
    // averaged
    int averaging = _avgperiod / _refresh;
    if (averaging == 0)
        averaging = 1;
    float value = pressure_ra.getAverageLast(
        averaging > pressure_ra.getCount() ? pressure_ra.getCount() : averaging);
    value = round(1. * value) / 1.;
    _alpacaServer->respond(request, value);
}

void ObservingConditions::aGetSkyTemperature(AsyncWebServerRequest *request) {
    // averaged
    int averaging = _avgperiod / _refresh;
    if (averaging == 0)
        averaging = 1;
    float value = tempsky_ra.getAverageLast(
        averaging > temperature_ra.getCount() ? tempsky_ra.getCount() : averaging);
    value = round(10. * value) / 10.;
    _alpacaServer->respond(request, value);
}

void ObservingConditions::aGetCloudCover(AsyncWebServerRequest *request) {
    // averaged
    int averaging = _avgperiod / _refresh;
    if (averaging == 0)
        averaging = 1;
    float value = cloudcover_ra.getAverageLast(
        averaging > cloudcover_ra.getCount() ? cloudcover_ra.getCount() : averaging);
    value = round(1. * value) / 1.;
    _alpacaServer->respond(request, value);
}

void ObservingConditions::aGetStarFwhm(AsyncWebServerRequest *request) {
    // averaged
    int averaging = _avgperiod / _refresh;
    if (averaging == 0)
        averaging = 1;
    float value = noisedb_ra.getAverageLast(
        averaging > noisedb_ra.getCount() ? noisedb_ra.getCount() : averaging);
    value = round(10. * value) / 10.;
    _alpacaServer->respond(request, value);
}

void ObservingConditions::aReadJson(JsonObject &root) {
    AlpacaObservingConditions::aReadJson(root);
    if (JsonObject obj_config = root[F("Configuration")]) {
        _avgperiod = obj_config[F("A_Average_Periodzc_sec")] | _avgperiod;
        _refresh = obj_config[F("B_Refresh_Periodzc_sec")] | _refresh;
    }
}

void ObservingConditions::aWriteJson(JsonObject &root) {
    AlpacaObservingConditions::aWriteJson(root);
    // read-only values marked with #
    JsonObject obj_config = root[F("Configuration")].to<JsonObject>();
    obj_config[F("A_Average_Periodzc_sec")] = _avgperiod;
    obj_config[F("B_Refresh_Periodzc_sec")] = _refresh;
    obj_config[F("Sensors_Descriptionzro")] = sensordescription;

    // instant
    JsonObject obj_instant_state = root[F("Instant State (Latest)")].to<JsonObject>();
    obj_instant_state[F("Rain_Rate,_mm/hzro")] = String(rainrate, 1);
    obj_instant_state[F("Temperature,_°Czro")] = String(temperature, 1);
    obj_instant_state[F("Humidity,_zpzro")] = String(humidity, 0);
    obj_instant_state[F("Dewpoint,_°Czro")] = String(dewpoint, 1);
    obj_instant_state[F("Pressure,_hPazro")] = String(pressure, 0);
    obj_instant_state[F("Sky_Temp,_°Czro")] = String(tempsky, 1);
    obj_instant_state[F("Cloud_Cover,_zpzro")] = String(cloudcover, 0);
    // not exactly seeing (fwhm)
    obj_instant_state[F("Turbulence,_dBzro")] = String(noisedb, 1);
    // obj_instant_state[F("Sky Quality")] = skyquality;
    // obj_instant_state[F("Sky Brightness")] = skybrightness;
    obj_instant_state[F("Updated,_secs/agozro")] = String(
        ((float)millis() - (float)timelastupdate) / 1000., 1);

    // averaged
    int averaging = _avgperiod / _refresh;
    if (averaging == 0)
        averaging = 1;
    JsonObject obj_averaged_state = root[F("Averaged State (ASCOM)")].to<JsonObject>();
    obj_averaged_state[F("Rain_Rate,_mm/hzro")] = String(
        rainrate_ra.getAverageLast(
            averaging > rainrate_ra.getCount() ? rainrate_ra.getCount() : averaging),
        1);
    obj_averaged_state[F("Temperature,_°Czro")] = String(
        temperature_ra.getAverageLast(
            averaging > temperature_ra.getCount() ? temperature_ra.getCount() : averaging),
        1);
    obj_averaged_state[F("Humidity,_zpzro")] = String(
        humidity_ra.getAverageLast(
            averaging > humidity_ra.getCount() ? humidity_ra.getCount() : averaging),
        0);
    obj_averaged_state[F("Dewpoint,_°Czro")] = String(
        dewpoint_ra.getAverageLast(
            averaging > dewpoint_ra.getCount() ? dewpoint_ra.getCount() : averaging),
        1);
    obj_averaged_state[F("Pressure,_hPazro")] = String(
        pressure_ra.getAverageLast(
            averaging > pressure_ra.getCount() ? pressure_ra.getCount() : averaging),
        0);
    obj_averaged_state[F("Sky_Temp,_°Czro")] = String(
        tempsky_ra.getAverageLast(
            averaging > tempsky_ra.getCount() ? tempsky_ra.getCount() : averaging),
        1);
    obj_averaged_state[F("Cloud_Cover,_zpzro")] = String(
        cloudcover_ra.getAverageLast(
            averaging > cloudcover_ra.getCount() ? cloudcover_ra.getCount() : averaging),
        0);
    // not exactly seeing (fwhm)
    obj_averaged_state[F("Turbulence,_dBzro")] = String(
        noisedb_ra.getAverageLast(
            averaging > noisedb_ra.getCount() ? noisedb_ra.getCount() : averaging),
        1);
    // obj_averaged_state[F("Sky Quality")] = String(
    //     skyquality_ra.getAverageLast(
    //         averaging > skyquality_ra.getCount() ? skyquality_ra.getCount() : averaging),
    //     1);
    // obj_averaged_state[F("Sky Brightness")] = String(
    //     skybrightness_ra.getAverageLast(
    //         averaging > skybrightness_ra.getCount() ? skybrightness_ra.getCount() : averaging),
    //     1);
    obj_averaged_state[F("Updated,_secs/agozro")] = String(
        ((float)millis() - (float)timelastupdate) / 1000., 1);
}