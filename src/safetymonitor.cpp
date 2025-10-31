#include "safetymonitor.h"
#include "meteo.h"

// cannot call member functions directly from interrupt, so need these helpers for up to 1 SafetyMonitor
uint8_t SafetyMonitor::_n_safetymonitors = 0;
SafetyMonitor *SafetyMonitor::_safetymonitor_array[4] = {nullptr, nullptr, nullptr, nullptr};

void SafetyMonitor::logMessage(String msg, bool showtime) {
    if (logLine && logLinePart) {
        if (logTime && showtime) {
            logLinePart(logTime() + " ");
        }
        logLine(msg);
    }
}

void SafetyMonitor::logMessagePart(String msg, bool showtime) {
    if (logLinePart) {
        if (logTime && showtime) {
            logLinePart(logTime() + " ");
        }
        logLinePart(msg);
    }
}

void SafetyMonitor::setLogger(std::function<void(String)> logLineCallback, std::function<void(String)> logLinePartCallback, std::function<String()> logTimeCallback) {
    logLine = logLineCallback;
    logLinePart = logLinePartCallback;
    logTime = logTimeCallback;
}

bool SafetyMonitor::begin() {
    _safetymonitor_array[_safetymonitor_index] = this;
    return true;
}

void SafetyMonitor::update(Meteo meteo, unsigned long measureDelay) {
    //  update meteo
    temperature = meteo.bmp_temperature;
    humidity = meteo.aht_humidity;
    pressure = meteo.bmp_pressure;
    dewpoint = meteo.dewpoint;
    tempsky = meteo.tempsky;

    if (temperature > limit_tamb) {
        status_tamb = true;
    } else {
        status_tamb = false;
    }
    if (humidity < limit_humid) {
        status_humid = true;
    } else {
        status_humid = false;
    }
    if (tempsky < limit_tsky) {
        status_tsky = true;
    } else {
        status_tsky = false;
    }
    if ((temperature - dewpoint) > limit_dew) {
        status_dew = true;
    } else {
        status_dew = false;
    }
    if (status_tamb && status_humid && status_tsky && status_dew) {
        instant_status = true;
    } else {
        instant_status = false;
    }
    if (instant_status == false) {
        if (status_weather == true)
            logMessage("[SAFEMON] Unsafe received");
        time2open = delay2open;
        status_weather = false;
        if (status_roof == true) {
            if (time2close == 0.) {
                status_roof = false;
                _issafe = false;
                logMessage("[SAFEMON] Close Roofs");
                digitalWrite(ROOFpin, LOW);
            }
        }
    }
    if (instant_status == true) {
        if (status_weather == false)
            logMessage("[SAFEMON] Safe received");
        time2close = delay2close;
        status_weather = true;
        if (status_roof == false) {
            if (time2open == 0.) {
                status_roof = true;
                _issafe = true;
                logMessage("[SAFEMON] Open Roofs");
                digitalWrite(ROOFpin, HIGH);
            }
        }
    }
    time2open -= measureDelay / 1000;
    if (time2open < 0.)
        time2open = 0;
    time2close -= measureDelay / 1000;
    if (time2close < 0.)
        time2close = 0;
};

void SafetyMonitor::aReadJson(JsonObject &root) {
    AlpacaSafetyMonitor::aReadJson(root);
    if (JsonObject obj_config = root[F("Configuration")]) {
        limit_tamb = obj_config[F("Freezing_Temperature")] | limit_tamb;
        limit_tsky = obj_config[F("Cloudy_SkyTemperature")] | limit_tsky;
        limit_humid = obj_config[F("Humidity_limit")] | limit_humid;
        limit_dew = obj_config[F("Dew_delta_Temperature")] | limit_dew;
        delay2open = obj_config[F("Delay_to_Open")] | delay2open;
        delay2close = obj_config[F("Delay_to_Close")] | delay2close;
    }
    status_roof = root[F("State")][F("Safety Monitor Status")] | status_roof;
}

void SafetyMonitor::aWriteJson(JsonObject &root) {
    AlpacaSafetyMonitor::aWriteJson(root);
    // read-only values marked with #
    JsonObject obj_config = root[F("Configuration")].to<JsonObject>();
    obj_config[F("Freezing_Temperature")] = limit_tamb;
    obj_config[F("Cloudy_SkyTemperature")] = limit_tsky;
    obj_config[F("Humidity_limit")] = limit_humid;
    obj_config[F("Dew_delta_Temperature")] = limit_dew;
    obj_config[F("Delay_to_Open")] = delay2open;
    obj_config[F("Delay_to_Close")] = delay2close;

    JsonObject obj_state = root[F("State")].to<JsonObject>();
    obj_state[F("Ambient_Temperature")] = temperature;
    obj_state[F("Sky_Temperature ")] = tempsky;
    obj_state[F("Humidity")] = humidity;
    obj_state[F("Pressure")] = pressure;
    obj_state[F("Time_to_open")] = time2open;
    obj_state[F("Time_to_close")] = time2close;
    obj_state[F("Safety_Monitor_status")] = status_roof;
}
