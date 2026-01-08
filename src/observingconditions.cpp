#include "observingconditions.h"
#include "hardware.h"
#include "helpers.h"
#include "meteo.h"

// cannot call member functions directly from interrupt, so need these helpers for up to 1 ObservingConditions
uint8_t ObservingConditions::_n_observingconditionss = 0;
ObservingConditions *ObservingConditions::_observingconditions_array[4] = {nullptr, nullptr, nullptr, nullptr};

void ObservingConditions::logMessage(String msg, bool showtime) {
    if (logLine && logLinePart) {
        if (logTime && showtime) {
            logLinePart(logTime() + " ", logSource);
        }
        logLine(msg, logSource);
    }
}

void ObservingConditions::logMessagePart(String msg, bool showtime) {
    if (logLinePart) {
        if (logTime && showtime) {
            logLinePart(logTime() + " ", logSource);
        }
        logLinePart(msg, logSource);
    }
}

void ObservingConditions::setLogger(const int logSrc, std::function<void(String, const int)> logLineCallback, std::function<void(String, const int)> logLinePartCallback, std::function<String()> logTimeCallback) {
    logSource = logSrc;
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

void ObservingConditions::update(Meteo* meteo) {
    String message = "[OBSERVING][DATA]";

    if (OBSCON_RAINRATE) {
        rainrate = meteo->sensors.rain_rate;
        rainrate_ra.add(rainrate);
        message += " RR:" + String(rainrate, 1) + "/" + String(rainrate_ra.getAverageLast(_averaging > rainrate_ra.getCount() ? rainrate_ra.getCount() : _averaging), 1);
    } else {
        rainrate = 0;
        rainrate_ra.add(rainrate);
        message += " RR:-";
    }

    if (OBSCON_TEMPERATURE) {
        temperature = meteo->sensors.temperature;
        temperature_ra.add(temperature);
        message += " T:" + String(temperature, 1) + "/" + String(temperature_ra.getAverageLast(_averaging > temperature_ra.getCount() ? temperature_ra.getCount() : _averaging), 1);
    } else {
        temperature = 0;
        temperature_ra.add(temperature);
        message += " T:-";
    }

    if (OBSCON_HUMIDITY) {
        humidity = meteo->sensors.humidity;
        humidity_ra.add(humidity);
        message += " H:" + String(humidity, 0) + "/" + String(humidity_ra.getAverageLast(_averaging > humidity_ra.getCount() ? humidity_ra.getCount() : _averaging), 0);
    } else {
        humidity = 0;
        humidity_ra.add(humidity);
        message += " H:-";
    }

    if (OBSCON_PRESSURE) {
        pressure = meteo->sensors.bmp_pressure;
        pressure_ra.add(pressure);
        message += " P:" + String(pressure, 0) + "/" + String(pressure_ra.getAverageLast(_averaging > pressure_ra.getCount() ? pressure_ra.getCount() : _averaging), 0);
    } else {
        pressure = 0;
        pressure_ra.add(pressure);
        message += " P:-";
    }

    if (OBSCON_DEWPOINT) {
        dewpoint = meteo->sensors.dew_point;
        dewpoint_ra.add(dewpoint);
        message += " DP:" + String(dewpoint, 1) + "/" + String(dewpoint_ra.getAverageLast(_averaging > dewpoint_ra.getCount() ? dewpoint_ra.getCount() : _averaging), 1);
    } else {
        dewpoint = 0;
        dewpoint_ra.add(dewpoint);
        message += " DP:-";
    }

    if (OBSCON_SKYTEMP) {
        skytemp = meteo->sensors.sky_temperature;
        skytemp_ra.add(skytemp);
        message += " ST:" + String(skytemp, 1) + "/" + String(skytemp_ra.getAverageLast(_averaging > skytemp_ra.getCount() ? skytemp_ra.getCount() : _averaging), 1);
    } else {
        skytemp = 0;
        skytemp_ra.add(skytemp);
        message += " ST:-";
    }

    if (OBSCON_FWHM) {
        noisedb = meteo->sensors.noise_db;
        noisedb_ra.add(noisedb);
        message += " TR:" + String(noisedb, 1) + "/" + String(noisedb_ra.getAverageLast(_averaging > noisedb_ra.getCount() ? noisedb_ra.getCount() : _averaging), 1);
    } else {
        noisedb = 0;
        noisedb_ra.add(noisedb);
        message += " TR:-";
    }

    if (OBSCON_CLOUDCOVER) {
        cloudcover = meteo->sensors.cloud_cover;
        cloudcover_ra.add(cloudcover);
        message += " CC:" + String(cloudcover, 0) + "/" + String(cloudcover_ra.getAverageLast(_averaging > cloudcover_ra.getCount() ? cloudcover_ra.getCount() : _averaging), 0);
    } else {
        cloudcover = 0;
        cloudcover_ra.add(cloudcover);
        message += " CC:-";
    }

    if (OBSCON_SKYQUALITY) {
        skyquality = meteo->sensors.sky_quality;
        skyquality_ra.add(skyquality);
        message += " SQ:" + String(skyquality, 1) + "/" + String(skyquality_ra.getAverageLast(_averaging > skyquality_ra.getCount() ? skyquality_ra.getCount() : _averaging), 1);
    } else {
        skyquality = 0;
        skyquality_ra.add(skyquality);
        message += " SQ:-";
    }

    if (OBSCON_SKYBRIGHTNESS) {
        skybrightness = meteo->sensors.sky_brightness;
        skybrightness_ra.add(skybrightness);
        message += " SB:" + smart_round(skybrightness) + "/" + smart_round(skybrightness_ra.getAverageLast(_averaging > skybrightness_ra.getCount() ? skybrightness_ra.getCount() : _averaging));
    } else {
        skybrightness = 0;
        skybrightness_ra.add(skybrightness);
        message += " SB:-";
    }

    if (OBSCON_WINDDIR) {
        winddir = meteo->sensors.wind_direction;
        winddir_ra.add(winddir);
        message += " WD:" + String(winddir, 1) + "/" + String(winddir_ra.getAverageLast(_averaging > winddir_ra.getCount() ? winddir_ra.getCount() : _averaging), 1);
    } else {
        winddir = 0;
        winddir_ra.add(winddir);
        message += " WD:-";
    }

    if (OBSCON_WINDSPEED) {
        windspeed = meteo->sensors.wind_speed;
        windspeed_ra.add(windspeed);
        message += " WS:" + String(windspeed, 1) + "/" + String(windspeed_ra.getAverageLast(_averaging > windspeed_ra.getCount() ? windspeed_ra.getCount() : _averaging), 1);
    } else {
        windspeed = 0;
        windspeed_ra.add(windspeed);
        message += " WS:-";
    }

    if (OBSCON_WINDGUST) {
        windgust = meteo->sensors.wind_gust;
        windgust_ra.add(windgust);
        message += " WG:" + String(windgust, 1) + "/" + String(windgust_ra.getAverageLast(_averaging > windgust_ra.getCount() ? windgust_ra.getCount() : _averaging), 1);
    } else {
        windgust = 0;
        windgust_ra.add(windgust);
        message += " WG:-";
    }

    timelastupdate = millis();

    if (logEnabled[LogSource::ObsCon] == Log::On || (logEnabled[LogSource::ObsCon] == Log::Slow && millis() - last_message > logSlow[LogSource::ObsCon] * 1000)) {
        logMessage(message);
        last_message = millis();
    }
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
    if (OBSCON_RAINRATE) {
        float value = rainrate_ra.getAverageLast(
            _averaging > rainrate_ra.getCount() ? rainrate_ra.getCount() : _averaging);
        value = round(10. * value) / 10.;
        _alpacaServer->respond(request, value);
    } else {
        _alpacaServer->respond(request, nullptr, AlpacaNotImplementedException, "Not Implemented");
    }
}

void ObservingConditions::aGetTemperature(AsyncWebServerRequest *request) {
    if (OBSCON_TEMPERATURE) {
        float value = temperature_ra.getAverageLast(
            _averaging > temperature_ra.getCount() ? temperature_ra.getCount() : _averaging);
        value = round(10. * value) / 10.;
        _alpacaServer->respond(request, value);
    } else {
        _alpacaServer->respond(request, nullptr, AlpacaNotImplementedException, "Not Implemented");
    }
}

void ObservingConditions::aGetHumidity(AsyncWebServerRequest *request) {
    if (OBSCON_HUMIDITY) {
        float value = humidity_ra.getAverageLast(
            _averaging > humidity_ra.getCount() ? humidity_ra.getCount() : _averaging);
        value = round(1. * value) / 1.;
        _alpacaServer->respond(request, value);
    } else {
        _alpacaServer->respond(request, nullptr, AlpacaNotImplementedException, "Not Implemented");
    }
}

void ObservingConditions::aGetDewPoint(AsyncWebServerRequest *request) {
    if (OBSCON_DEWPOINT) {
        float value = dewpoint_ra.getAverageLast(
            _averaging > dewpoint_ra.getCount() ? dewpoint_ra.getCount() : _averaging);
        value = round(10. * value) / 10.;
        _alpacaServer->respond(request, value);
    } else {
        _alpacaServer->respond(request, nullptr, AlpacaNotImplementedException, "Not Implemented");
    }
}

void ObservingConditions::aGetPressure(AsyncWebServerRequest *request) {
    if (OBSCON_PRESSURE) {
        float value = pressure_ra.getAverageLast(
            _averaging > pressure_ra.getCount() ? pressure_ra.getCount() : _averaging);
        value = round(1. * value) / 1.;
        _alpacaServer->respond(request, value);
    } else {
        _alpacaServer->respond(request, nullptr, AlpacaNotImplementedException, "Not Implemented");
    }
}

void ObservingConditions::aGetSkyTemperature(AsyncWebServerRequest *request) {
    if (OBSCON_SKYTEMP) {
        float value = skytemp_ra.getAverageLast(
            _averaging > skytemp_ra.getCount() ? skytemp_ra.getCount() : _averaging);
        value = round(10. * value) / 10.;
        _alpacaServer->respond(request, value);
    } else {
        _alpacaServer->respond(request, nullptr, AlpacaNotImplementedException, "Not Implemented");
    }
}

void ObservingConditions::aGetCloudCover(AsyncWebServerRequest *request) {
    if (OBSCON_CLOUDCOVER) {
        float value = cloudcover_ra.getAverageLast(
            _averaging > cloudcover_ra.getCount() ? cloudcover_ra.getCount() : _averaging);
        value = round(1. * value) / 1.;
        _alpacaServer->respond(request, value);
    } else {
        _alpacaServer->respond(request, nullptr, AlpacaNotImplementedException, "Not Implemented");
    }
}

void ObservingConditions::aGetStarFwhm(AsyncWebServerRequest *request) {
    if (_noise_as_fwhm && OBSCON_FWHM) {
        float value = noisedb_ra.getAverageLast(
            _averaging > noisedb_ra.getCount() ? noisedb_ra.getCount() : _averaging);
        value = round(10. * value) / 10.;
        _alpacaServer->respond(request, value);
    } else {
        _alpacaServer->respond(request, nullptr, AlpacaNotImplementedException, "Not Implemented");
    }
}

void ObservingConditions::aGetSkyBrightness(AsyncWebServerRequest *request) {
    if (OBSCON_SKYBRIGHTNESS) {
        float value = skybrightness_ra.getAverageLast(
            _averaging > skybrightness_ra.getCount() ? skybrightness_ra.getCount() : _averaging);
        value = round(10. * value) / 10.;
        _alpacaServer->respond(request, value);
    } else {
        _alpacaServer->respond(request, nullptr, AlpacaNotImplementedException, "Not Implemented");
    }
}

void ObservingConditions::aGetSkyQuality(AsyncWebServerRequest *request) {
    if (OBSCON_SKYQUALITY) {
        float value = skyquality_ra.getAverageLast(
            _averaging > skyquality_ra.getCount() ? skyquality_ra.getCount() : _averaging);
        value = round(10. * value) / 10.;
        _alpacaServer->respond(request, value);
    } else {
        _alpacaServer->respond(request, nullptr, AlpacaNotImplementedException, "Not Implemented");
    }
}

void ObservingConditions::aGetWindDirection(AsyncWebServerRequest *request) {
    if (OBSCON_WINDDIR) {
        float value = winddir_ra.getAverageLast(
            _averaging > winddir_ra.getCount() ? winddir_ra.getCount() : _averaging);
        value = round(1. * value) / 1.;
        _alpacaServer->respond(request, value);
    } else {
        _alpacaServer->respond(request, nullptr, AlpacaNotImplementedException, "Not Implemented");
    }
}

void ObservingConditions::aGetWindGust(AsyncWebServerRequest *request) {
    if (OBSCON_WINDGUST) {
        // Wind gust not averaged, ASCOM (https://ascom-standards.org/newdocs/observingconditions.html#ObservingConditions.WindGust)
        float value = windgust;
        value = round(10. * value) / 10.;
        _alpacaServer->respond(request, value);
    } else {
        _alpacaServer->respond(request, nullptr, AlpacaNotImplementedException, "Not Implemented");
    }
}

void ObservingConditions::aGetWindSpeed(AsyncWebServerRequest *request) {
    if (OBSCON_WINDSPEED) {
        float value = windspeed_ra.getAverageLast(
            _averaging > windspeed_ra.getCount() ? windspeed_ra.getCount() : _averaging);
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
    obj_instant_state[F("Rain_Rate,_mm/hzro")] = OBSCON_RAINRATE ? String(rainrate, 1) : "n/a";
    obj_instant_state[F("Temperature,_°Czro")] = OBSCON_TEMPERATURE ? String(temperature, 1) : "n/a";
    obj_instant_state[F("Humidity,_zpzro")] = OBSCON_HUMIDITY ? String(humidity, 0) : "n/a";
    obj_instant_state[F("Dewpoint,_°Czro")] = OBSCON_DEWPOINT ? String(dewpoint, 1) : "n/a";
    obj_instant_state[F("Pressure,_hPazro")] = OBSCON_PRESSURE ? String(pressure, 0) : "n/a";
    obj_instant_state[F("Sky_Temp,_°Czro")] = OBSCON_SKYTEMP ? String(skytemp, 1) : "n/a";
    obj_instant_state[F("Cloud_Cover,_zpzro")] = OBSCON_CLOUDCOVER ? String(cloudcover, 0) : "n/a";
    // not exactly seeing (fwhm)
    obj_instant_state[F("Turbulence,_dBzro")] = OBSCON_FWHM ? String(noisedb, 1) : "n/a";
    obj_instant_state[F("Sky_Quality,_m/saszro")] = OBSCON_SKYQUALITY ? String(skyquality, 1) : "n/a";
    obj_instant_state[F("Sky_Brightness,_luxzro")] = OBSCON_SKYBRIGHTNESS ? smart_round(skybrightness) : "n/a";
    obj_instant_state[F("Wind_Direction,_°zro")] = OBSCON_WINDDIR ? String(winddir, 0) : "n/a";
    obj_instant_state[F("Wind_Speed,_m/szro")] = OBSCON_WINDSPEED ? String(windspeed, 1) : "n/a";
    obj_instant_state[F("Wind_Gust,_m/szro")] = OBSCON_WINDGUST ? String(windgust, 1) : "n/a";
    obj_instant_state[F("Updated,_secs/agozro")] = String(((float)millis() - (float)timelastupdate) / 1000., 1);

    // averaged
    JsonObject obj_averaged_state = root[F("Averaged State (ASCOM)")].to<JsonObject>();
    obj_averaged_state[F("Rain_Rate,_mm/hzro")] = OBSCON_RAINRATE ? String(rainrate_ra.getAverageLast(_averaging > rainrate_ra.getCount() ? rainrate_ra.getCount() : _averaging), 1) : "n/a";
    obj_averaged_state[F("Temperature,_°Czro")] = OBSCON_TEMPERATURE ? String(temperature_ra.getAverageLast(_averaging > temperature_ra.getCount() ? temperature_ra.getCount() : _averaging), 1) : "n/a";
    obj_averaged_state[F("Humidity,_zpzro")] = OBSCON_HUMIDITY ? String(humidity_ra.getAverageLast(_averaging > humidity_ra.getCount() ? humidity_ra.getCount() : _averaging), 0) : "n/a";
    obj_averaged_state[F("Dewpoint,_°Czro")] = OBSCON_DEWPOINT ? String(dewpoint_ra.getAverageLast(_averaging > dewpoint_ra.getCount() ? dewpoint_ra.getCount() : _averaging), 1) : "n/a";
    obj_averaged_state[F("Pressure,_hPazro")] = OBSCON_PRESSURE ? String(pressure_ra.getAverageLast(_averaging > pressure_ra.getCount() ? pressure_ra.getCount() : _averaging), 0) : "n/a";
    obj_averaged_state[F("Sky_Temp,_°Czro")] = OBSCON_SKYTEMP ? String(skytemp_ra.getAverageLast(_averaging > skytemp_ra.getCount() ? skytemp_ra.getCount() : _averaging), 1) : "n/a";
    obj_averaged_state[F("Cloud_Cover,_zpzro")] = OBSCON_CLOUDCOVER ? String(cloudcover_ra.getAverageLast(_averaging > cloudcover_ra.getCount() ? cloudcover_ra.getCount() : _averaging), 0) : "n/a";
    // not exactly seeing (fwhm)
    obj_averaged_state[F("Turbulence,_dBzro")] = OBSCON_FWHM ? String(noisedb_ra.getAverageLast(_averaging > noisedb_ra.getCount() ? noisedb_ra.getCount() : _averaging), 1) : "n/a";
    obj_averaged_state[F("Sky_Quality,_m/saszro")] = OBSCON_SKYQUALITY ? String(skyquality_ra.getAverageLast(_averaging > skyquality_ra.getCount() ? skyquality_ra.getCount() : _averaging), 1) : "n/a";
    obj_averaged_state[F("Sky_Brightness,_luxzro")] = OBSCON_SKYBRIGHTNESS ? smart_round(skybrightness_ra.getAverageLast(_averaging > skybrightness_ra.getCount() ? skybrightness_ra.getCount() : _averaging)) : "n/a";
    obj_averaged_state[F("Wind_Direction,_°zro")] = OBSCON_WINDDIR ? String(winddir_ra.getAverageLast(_averaging > winddir_ra.getCount() ? winddir_ra.getCount() : _averaging), 1) : "n/a";
    obj_averaged_state[F("Wind_Speed,_m/szro")] = OBSCON_WINDSPEED ? String(windspeed_ra.getAverageLast(_averaging > windspeed_ra.getCount() ? windspeed_ra.getCount() : _averaging), 1) : "n/a";
    // Wind gust not averaged, ASCOM (https://ascom-standards.org/newdocs/observingconditions.html#ObservingConditions.WindGust)
    obj_averaged_state[F("Wind_Gust,_m/szro")] = OBSCON_WINDGUST ? String(windgust, 1) : "n/a";
    obj_averaged_state[F("Updated,_secs/agozro")] = String(((float)millis() - (float)timelastupdate) / 1000., 1);
}