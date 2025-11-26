#include "observingconditions.h"
#include "meteo.h"

// cannot call member functions directly from interrupt, so need these helpers for up to 1 ObservingConditions
uint8_t ObservingConditions::_n_observingconditionss = 0;
ObservingConditions *ObservingConditions::_observingconditions_array[4] = {nullptr, nullptr, nullptr, nullptr};

void ObservingConditions::logMessage(String msg, bool showtime) {
    if (logLine && logLinePart) {
        if (logTime && showtime) {
            logLinePart(logTime() + " ");
        }
        logLine(msg);
    }
}

void ObservingConditions::logMessagePart(String msg, bool showtime) {
    if (logLinePart) {
        if (logTime && showtime) {
            logLinePart(logTime() + " ");
        }
        logLinePart(msg);
    }
}

void ObservingConditions::setLogger(std::function<void(String)> logLineCallback, std::function<void(String)> logLinePartCallback, std::function<String()> logTimeCallback) {
    logLine = logLineCallback;
    logLinePart = logLinePartCallback;
    logTime = logTimeCallback;
}

void ObservingConditions::setImmediateUpdate(std::function<void()> immediateUpdateCallcback) {
    immediateUpdate = immediateUpdateCallcback;
}

bool ObservingConditions::begin() {
    _observingconditions_array[_observingconditions_index] = this;
    _averaging = _avgperiod / _refresh;
    if (_averaging == 0) {
        _averaging = 1;
    }
    return true;
}

void ObservingConditions::update(Meteo meteo) {
    String message = "[OBSERVING][DATA]";

    rainrate = meteo.rainrate;
    rainrate_ra.add(rainrate);
    message += " RR:" + String(rainrate, 1) + "/" + String(rainrate_ra.getAverageLast(_averaging > rainrate_ra.getCount() ? rainrate_ra.getCount() : _averaging), 1);

    temperature = (meteo.bmp_temperature + meteo.aht_temperature) / 2;
    temperature_ra.add(temperature);
    message += " T:" + String(temperature, 1) + "/" + String(temperature_ra.getAverageLast(_averaging > temperature_ra.getCount() ? temperature_ra.getCount() : _averaging), 1);

    humidity = meteo.aht_humidity;
    humidity_ra.add(humidity);
    message += " H:" + String(humidity, 0) + "/" + String(humidity_ra.getAverageLast(_averaging > humidity_ra.getCount() ? humidity_ra.getCount() : _averaging), 0);

    pressure = meteo.bmp_pressure;
    pressure_ra.add(pressure);
    message += " P:" + String(pressure, 0) + "/" + String(pressure_ra.getAverageLast(_averaging > pressure_ra.getCount() ? pressure_ra.getCount() : _averaging), 0);

    dewpoint = meteo.dewpoint;
    dewpoint_ra.add(dewpoint);
    message += " DP:" + String(dewpoint, 1) + "/" + String(dewpoint_ra.getAverageLast(_averaging > dewpoint_ra.getCount() ? dewpoint_ra.getCount() : _averaging), 1);

    tempsky = meteo.tempsky;
    tempsky_ra.add(tempsky);
    message += " ST:" + String(tempsky, 1) + "/" + String(tempsky_ra.getAverageLast(_averaging > tempsky_ra.getCount() ? tempsky_ra.getCount() : _averaging), 1);

    noisedb = meteo.noise_db;
    noisedb_ra.add(noisedb);
    message += " TR:" + String(noisedb, 1) + "/" + String(noisedb_ra.getAverageLast(_averaging > noisedb_ra.getCount() ? noisedb_ra.getCount() : _averaging), 1);

    cloudcover = meteo.cloudcover;
    cloudcover_ra.add(cloudcover);
    message += " CC:" + String(cloudcover, 0) + "/" + String(cloudcover_ra.getAverageLast(_averaging > cloudcover_ra.getCount() ? cloudcover_ra.getCount() : _averaging), 0);

    skyquality = meteo.skyquality;
    skyquality_ra.add(skyquality);

    skybrightness = meteo.skybrightness;
    skybrightness_ra.add(skybrightness);

    timelastupdate = millis();

    logMessage(message);
};

void ObservingConditions::aGetDescription(AsyncWebServerRequest *request) {
    String description = "DreamSky Observing Conditions Monitor";
    _alpacaServer->respond(request, description.c_str());
}

void ObservingConditions::aGetDriverVersion(AsyncWebServerRequest *request) {
    String version = "​" + String(VERSION) + ", build " + String(BUILD_NUMBER);
    _alpacaServer->respond(request, version.c_str());
};

void ObservingConditions::aGetDriverInfo(AsyncWebServerRequest *request) {
    String year = String(BUILD_DATE).substring(0, 4);
    String info = "©" + year + " DreamSky Observatory";
    _alpacaServer->respond(request, info.c_str());
};

void ObservingConditions::aGetTimeSinceLastUpdate(AsyncWebServerRequest *request) {
    float seconds = (millis() - timelastupdate) / 1000;
    _alpacaServer->respond(request, seconds);
}

void ObservingConditions::aGetAveragePeriod(AsyncWebServerRequest *request) {
    // ASCOM required hours
    float value = (float)_avgperiod / 3600.;
    _alpacaServer->respond(request, value);
}

void ObservingConditions::aGetRainRate(AsyncWebServerRequest *request) {
    float value = rainrate_ra.getAverageLast(
        _averaging > rainrate_ra.getCount() ? rainrate_ra.getCount() : _averaging);
    value = round(10. * value) / 10.;
    _alpacaServer->respond(request, value);
}

void ObservingConditions::aGetTemperature(AsyncWebServerRequest *request) {
    float value = temperature_ra.getAverageLast(
        _averaging > temperature_ra.getCount() ? temperature_ra.getCount() : _averaging);
    value = round(10. * value) / 10.;
    _alpacaServer->respond(request, value);
}

void ObservingConditions::aGetHumidity(AsyncWebServerRequest *request) {
    float value = humidity_ra.getAverageLast(
        _averaging > humidity_ra.getCount() ? humidity_ra.getCount() : _averaging);
    value = round(1. * value) / 1.;
    _alpacaServer->respond(request, value);
}

void ObservingConditions::aGetDewPoint(AsyncWebServerRequest *request) {
    float value = dewpoint_ra.getAverageLast(
        _averaging > dewpoint_ra.getCount() ? dewpoint_ra.getCount() : _averaging);
    value = round(10. * value) / 10.;
    _alpacaServer->respond(request, value);
}

void ObservingConditions::aGetPressure(AsyncWebServerRequest *request) {
    float value = pressure_ra.getAverageLast(
        _averaging > pressure_ra.getCount() ? pressure_ra.getCount() : _averaging);
    value = round(1. * value) / 1.;
    _alpacaServer->respond(request, value);
}

void ObservingConditions::aGetSkyTemperature(AsyncWebServerRequest *request) {
    float value = tempsky_ra.getAverageLast(
        _averaging > temperature_ra.getCount() ? tempsky_ra.getCount() : _averaging);
    value = round(10. * value) / 10.;
    _alpacaServer->respond(request, value);
}

void ObservingConditions::aGetCloudCover(AsyncWebServerRequest *request) {
    float value = cloudcover_ra.getAverageLast(
        _averaging > cloudcover_ra.getCount() ? cloudcover_ra.getCount() : _averaging);
    value = round(1. * value) / 1.;
    _alpacaServer->respond(request, value);
}

void ObservingConditions::aGetStarFwhm(AsyncWebServerRequest *request) {
    if (_noise_as_fwhm) {
        float value = noisedb_ra.getAverageLast(
            _averaging > noisedb_ra.getCount() ? noisedb_ra.getCount() : _averaging);
        value = round(10. * value) / 10.;
        _alpacaServer->respond(request, value);
    } else {
        _alpacaServer->respond(request, nullptr, AlpacaNotImplementedException, "Not Implemented");
    }
}

void ObservingConditions::aReadJson(JsonObject &root) {
    AlpacaObservingConditions::aReadJson(root);
    if (JsonObject obj_config = root[F("Configuration")]) {
        _avgperiod = obj_config[F("A_Average_Periodzc_sec")] | _avgperiod;
        _refresh = obj_config[F("B_Refresh_Periodzc_sec")] | _refresh;
        if (_refresh < 3) {
            _refresh = 3;
        }
        _averaging = _avgperiod / _refresh;
        if (_averaging == 0) {
            _averaging = 1;
        }
        if (obj_config[F("D_Turbulence_as_FWHM")].as<String>() == String("true"))
            _noise_as_fwhm = true;
        else
            _noise_as_fwhm = false;
    }
}

void ObservingConditions::aWriteJson(JsonObject &root) {
    AlpacaObservingConditions::aWriteJson(root);
    // read-only values marked with #
    JsonObject obj_config = root[F("Configuration")].to<JsonObject>();
    obj_config[F("A_Average_Periodzc_sec")] = _avgperiod;
    obj_config[F("B_Refresh_Periodzc_sec")] = _refresh;
    obj_config[F("C_Averagingzc_countszro")] = _averaging;
    obj_config[F("D_Turbulence_as_FWHM")] = _noise_as_fwhm;
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
    // obj_instant_state[F("Sky Quality")] = String(skyquality, 1);
    // obj_instant_state[F("Sky Brightness")] = skybrightness;
    obj_instant_state[F("Updated,_secs/agozro")] = String(
        ((float)millis() - (float)timelastupdate) / 1000., 1);

    // averaged
    JsonObject obj_averaged_state = root[F("Averaged State (ASCOM)")].to<JsonObject>();
    obj_averaged_state[F("Rain_Rate,_mm/hzro")] = String(
        rainrate_ra.getAverageLast(
            _averaging > rainrate_ra.getCount() ? rainrate_ra.getCount() : _averaging),
        1);
    obj_averaged_state[F("Temperature,_°Czro")] = String(
        temperature_ra.getAverageLast(
            _averaging > temperature_ra.getCount() ? temperature_ra.getCount() : _averaging),
        1);
    obj_averaged_state[F("Humidity,_zpzro")] = String(
        humidity_ra.getAverageLast(
            _averaging > humidity_ra.getCount() ? humidity_ra.getCount() : _averaging),
        0);
    obj_averaged_state[F("Dewpoint,_°Czro")] = String(
        dewpoint_ra.getAverageLast(
            _averaging > dewpoint_ra.getCount() ? dewpoint_ra.getCount() : _averaging),
        1);
    obj_averaged_state[F("Pressure,_hPazro")] = String(
        pressure_ra.getAverageLast(
            _averaging > pressure_ra.getCount() ? pressure_ra.getCount() : _averaging),
        0);
    obj_averaged_state[F("Sky_Temp,_°Czro")] = String(
        tempsky_ra.getAverageLast(
            _averaging > tempsky_ra.getCount() ? tempsky_ra.getCount() : _averaging),
        1);
    obj_averaged_state[F("Cloud_Cover,_zpzro")] = String(
        cloudcover_ra.getAverageLast(
            _averaging > cloudcover_ra.getCount() ? cloudcover_ra.getCount() : _averaging),
        0);
    // not exactly seeing (fwhm)
    obj_averaged_state[F("Turbulence,_dBzro")] = String(
        noisedb_ra.getAverageLast(
            _averaging > noisedb_ra.getCount() ? noisedb_ra.getCount() : _averaging),
        1);
    // obj_averaged_state[F("Sky Quality")] = String(
    //     skyquality_ra.getAverageLast(
    //         _averaging > skyquality_ra.getCount() ? skyquality_ra.getCount() : _averaging),
    //     1);
    // obj_averaged_state[F("Sky Brightness")] = String(
    //     skybrightness_ra.getAverageLast(
    //         _averaging > skybrightness_ra.getCount() ? skybrightness_ra.getCount() : _averaging),
    //     1);
    obj_averaged_state[F("Updated,_secs/agozro")] = String(
        ((float)millis() - (float)timelastupdate) / 1000., 1);
}