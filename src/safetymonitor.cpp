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

int SafetyMonitor::getRainRateCountdown() {
    if (rainrate_state == RainRateState::AWAIT_DRY) {
        if (rainrate_occur == 0)
            return 0;
        int countdown = static_cast<int>(std::round(((1000. * rainrate_dry_delay) - (millis() - rainrate_occur))) / 1000.);
        if (countdown < 0)
            return 0;
        return countdown;
    }
    if (rainrate_state == RainRateState::AWAIT_WET) {
        if (rainrate_occur == 0)
            return 0;
        int countdown = static_cast<int>(std::round(((1000. * rainrate_wet_delay) - (millis() - rainrate_occur))) / 1000.);
        if (countdown < 0)
            return 0;
        return countdown;
    }
    return 0;
}

int SafetyMonitor::getSafeUnsafeCountdown() {
    if (safeunsafe_state == SafeUnsafeStatus::AWAIT_SAFE) {
        if (safeunsafe_occur == 0)
            return 0;
        int countdown = static_cast<int>(std::round(((1000. * safe_delay) - (millis() - safeunsafe_occur))) / 1000.);
        if (countdown < 0)
            return 0;
        return countdown;
    }
    if (safeunsafe_state == SafeUnsafeStatus::AWAIT_UNSAFE) {
        if (safeunsafe_occur == 0)
            return 0;
        int countdown = static_cast<int>(std::round(((1000. * unsafe_delay) - (millis() - safeunsafe_occur))) / 1000.);
        if (countdown < 0)
            return 0;
        return countdown;
    }
    return 0;
}

bool SafetyMonitor::begin() {
    _safetymonitor_array[_safetymonitor_index] = this;
    return true;
}

void SafetyMonitor::update(Meteo meteo) {
    //  update meteo
    temperature = (meteo.bmp_temperature + meteo.aht_temperature) / 2;
    humidity = meteo.aht_humidity;
    dewpoint = meteo.dew_point;
    dewpoint_delta = (temperature - dewpoint > 0 ? temperature - dewpoint : 0);
    tempsky = meteo.sky_temperature;
    // windspeed = meteo.windspeed;
    if (!rain_init) {
        rainrate = meteo.rain_rate;
        if (rainrate > 0) {
            rainrate_prev = rainrate;
            rainrate_curr = rainrate;
            rainrate_state = RainRateState::WET;
        } else {
            rainrate_prev = 0;
            rainrate_curr = 0;
            rainrate_state = RainRateState::DRY;
        }
        rain_init = true;
    }
    if (meteo.rain_rate > 0) {
        rainrate_curr = meteo.rain_rate;
    } else {
        rainrate_curr = 0;
    }
    if (rainrate_curr > 0 && rainrate_prev == 0) {
        rainrate_state = RainRateState::AWAIT_WET;
        rainrate_occur = millis();
    }
    if (rainrate_curr == 0 && rainrate_prev > 0) {
        rainrate_state = RainRateState::AWAIT_DRY;
        rainrate_occur = millis();
    }
    rainrate_prev = rainrate_curr;
    if (millis() - rainrate_occur >= rainrate_dry_delay * 1000 && rainrate_state == RainRateState::AWAIT_DRY) {
        rainrate_occur = 0;
        rainrate_state = RainRateState::DRY;
        rainrate = 0;
    }
    if (millis() - rainrate_occur >= rainrate_wet_delay * 1000 && rainrate_state == RainRateState::AWAIT_WET) {
        rainrate_occur = 0;
        rainrate_state = RainRateState::WET;
        rainrate = rainrate_curr;
    }
    // Safety
    rain_safe = (rain_prove ? (rainrate_state == RainRateState::DRY || rainrate_state == RainRateState::AWAIT_WET ? true : false) : true);
    temp_safe = (temp_prove ? (temperature > temp_upper_limit ? true : (temperature <= temp_lower_limit ? false : temp_safe)) : true);
    humi_safe = (humi_prove ? (humidity < humi_lower_limit ? true : (humidity >= humi_lower_limit ? false : humi_safe)) : true);
    dewdelta_safe = (dewdelta_prove ? (dewpoint_delta > dewdelta_upper_limit ? true : (dewpoint_delta <= dewdelta_lower_limit ? false : dewdelta_safe)) : true);
    skytemp_safe = (skytemp_prove ? (tempsky < skytemp_lower_limit ? true : (tempsky >= skytemp_upper_limit ? false : skytemp_safe)) : true);
    wind_safe = (wind_prove ? (windspeed < wind_lower_limit ? true : (windspeed >= wind_upper_limit ? false : wind_safe)) : true);

    // is_safe = rain_safe && temp_safe && humi_safe && dewdelta_safe && skytemp_safe && wind_safe;
    safeunsafe_curr = rain_safe && temp_safe && humi_safe && dewdelta_safe && skytemp_safe && wind_safe;
    if (!safeunsafe_init) {
        safeunsafe_prev = safeunsafe_curr;
        if (safeunsafe_curr) {
            safeunsafe_state = SafeUnsafeStatus::SAFE;
        } else {
            safeunsafe_state = SafeUnsafeStatus::UNSAFE;
        }
        safeunsafe_init = true;
    }
    if (safeunsafe_curr && !safeunsafe_prev) {
        safeunsafe_state = SafeUnsafeStatus::AWAIT_SAFE;
        safeunsafe_occur = millis();
    }
    if (!safeunsafe_curr && safeunsafe_prev) {
        safeunsafe_state = SafeUnsafeStatus::AWAIT_UNSAFE;
        safeunsafe_occur = millis();
    }
    safeunsafe_prev = safeunsafe_curr;
    if (millis() - safeunsafe_occur >= safe_delay * 1000 && safeunsafe_state == SafeUnsafeStatus::AWAIT_SAFE) {
        safeunsafe_occur = 0;
        safeunsafe_state = SafeUnsafeStatus::SAFE;
    }
    if (millis() - safeunsafe_occur >= unsafe_delay * 1000 && safeunsafe_state == SafeUnsafeStatus::AWAIT_UNSAFE) {
        safeunsafe_occur = 0;
        safeunsafe_state = SafeUnsafeStatus::UNSAFE;
    }
    is_safe = (safeunsafe_state == SafeUnsafeStatus::AWAIT_UNSAFE || safeunsafe_state == SafeUnsafeStatus::SAFE);
};

void SafetyMonitor::aGetDescription(AsyncWebServerRequest *request) {
    String description = "DreamSky Safety Conditions Monitor";
    _alpacaServer->respond(request, description.c_str());
}

void SafetyMonitor::aGetDriverVersion(AsyncWebServerRequest *request) {
    String version = "​" + String(VERSION) + ", build " + String(BUILD_NUMBER);
    _alpacaServer->respond(request, version.c_str());
};

void SafetyMonitor::aGetDriverInfo(AsyncWebServerRequest *request) {
    String year = String(BUILD_DATE).substring(0, 4);
    String info = "©" + year + " DreamSky Observatory";
    _alpacaServer->respond(request, info.c_str());
};

void SafetyMonitor::aReadJson(JsonObject &root) {
    AlpacaSafetyMonitor::aReadJson(root);
    if (JsonObject obj_config = root[F("Configuration")]) {
        // Safe Monitor
        safe_delay = obj_config[F("B___zda_safe_delayzc_s")] | safe_delay;
        unsafe_delay = obj_config[F("C___zda_unsafe_delayzc_s")] | unsafe_delay;
        // Rain
        if (obj_config[F("D_Rain_Ratezc_prove")].as<String>() == String("true"))
            rain_prove = true;
        else
            rain_prove = false;
        rainrate_wet_delay = obj_config[F("E___zda_wet_delayzc_s")] | rainrate_wet_delay;
        rainrate_dry_delay = obj_config[F("F___zda_dry_delayzc_s")] | rainrate_dry_delay;
        // Temp
        if (obj_config[F("G_Temperaturezc_prove")].as<String>() == String("true"))
            temp_prove = true;
        else
            temp_prove = false;
        temp_lower_limit = obj_config[F("H___zda_lower_limitzc_zdgC")] | temp_lower_limit;
        temp_upper_limit = obj_config[F("I___zda_upper_limitzc_zdgC")] | temp_upper_limit;
        // Humi
        if (obj_config[F("J_Humidityzc_prove")].as<String>() == String("true"))
            humi_prove = true;
        else
            humi_prove = false;
        humi_lower_limit = obj_config[F("K___zda_lower_limitzc_zdgC")] | humi_lower_limit;
        humi_upper_limit = obj_config[F("L___zda_upper_limitzc_zdgC")] | humi_upper_limit;
        // DewDelta
        if (obj_config[F("M_Dew_Point_zdtzc_prove")].as<String>() == String("true"))
            dewdelta_prove = true;
        else
            dewdelta_prove = false;
        dewdelta_lower_limit = obj_config[F("N___zda_lower_limitzc_zdgC")] | dewdelta_lower_limit;
        dewdelta_upper_limit = obj_config[F("O___zda_upper_limitzc_zdgC")] | dewdelta_upper_limit;
        // SkyTemp
        if (obj_config[F("P_Sky_Tempzc_prove")].as<String>() == String("true"))
            skytemp_prove = true;
        else
            skytemp_prove = false;
        skytemp_lower_limit = obj_config[F("Q___zda_lower_limitzc_zdgC")] | skytemp_lower_limit;
        skytemp_upper_limit = obj_config[F("R___zda_upper_limitzc_zdgC")] | skytemp_upper_limit;
        // WindSpeed
        if (obj_config[F("S_Wind_Speedzc_prove")].as<String>() == String("true"))
            wind_prove = true;
        else
            wind_prove = false;
        wind_lower_limit = obj_config[F("T___zda_lower_limitzc_zdgC")] | wind_lower_limit;
        wind_upper_limit = obj_config[F("U___zda_upper_limitzc_zdgC")] | wind_upper_limit;
    }
    // Manual for testing?
    // status_roof = root[F("State")][F("Safety Monitor Status")] | status_roof;
}

void SafetyMonitor::aWriteJson(JsonObject &root) {
    AlpacaSafetyMonitor::aWriteJson(root);
    // Configuration
    JsonObject obj_config = root[F("Configuration")].to<JsonObject>();
    obj_config[F("A_Safe_Monitorzc_provezro")] = true;
    obj_config[F("B___zda_safe_delayzc_s")] = safe_delay;
    obj_config[F("C___zda_unsafe_delayzc_s")] = unsafe_delay;
    obj_config[F("D_Rain_Ratezc_prove")] = rain_prove;
    obj_config[F("E___zda_wet_delayzc_s")] = rainrate_wet_delay;
    obj_config[F("F___zda_dry_delayzc_s")] = rainrate_dry_delay;
    obj_config[F("G_Temperaturezc_prove")] = temp_prove;
    obj_config[F("H___zda_lower_limitzc_zdgC")] = temp_lower_limit;
    obj_config[F("I___zda_upper_limitzc_zdgC")] = temp_upper_limit;
    obj_config[F("J_Humidityzc_prove")] = humi_prove;
    obj_config[F("K___zda_lower_limitzc_zdgC")] = humi_lower_limit;
    obj_config[F("L___zda_upper_limitzc_zdgC")] = humi_upper_limit;
    obj_config[F("M_Dew_Point_zdtzc_prove")] = dewdelta_prove;
    obj_config[F("N___zda_lower_limitzc_zdgC")] = dewdelta_lower_limit;
    obj_config[F("O___zda_upper_limitzc_zdgC")] = dewdelta_upper_limit;
    obj_config[F("P_Sky_Tempzc_prove")] = skytemp_prove;
    obj_config[F("Q___zda_lower_limitzc_zdgC")] = skytemp_lower_limit;
    obj_config[F("R___zda_upper_limitzc_zdgC")] = skytemp_upper_limit;
    obj_config[F("S_Wind_Speedzc_prove")] = wind_prove;
    obj_config[F("T___zda_lower_limitzc_mzss")] = wind_lower_limit;
    obj_config[F("U___zda_upper_limitzc_zdgC")] = wind_upper_limit;
    // State
    // 🟢 🟡 🔵 🔴 ⚫ "⠀"
    // 🟩 🟨 🟦 🟥 ⬛
    JsonObject obj_state = root[F("State")].to<JsonObject>();
    String rain_state_icon = "⚫";
    String indent = "";
    if (rain_prove) {
        switch (rainrate_state) {
        case RainRateState::AWAIT_DRY:
            rain_state_icon = "🔴🟡";
            indent = "⠀⠀⠀⠀";
            break;
        case RainRateState::AWAIT_WET:
            rain_state_icon = "🟢🟡";
            indent = "⠀⠀⠀⠀";
            break;
        case RainRateState::DRY:
            rain_state_icon = "🟢";
            indent = "⠀⠀";
            break;
        case RainRateState::WET:
            rain_state_icon = "🔴";
            indent = "⠀⠀";
            break;
        }
    }
    obj_state[rain_state_icon + ("_Rainzro")] = !rain_safe;
    obj_state[indent + "_Rate,_mmzshzro"] = String(rainrate, 1);
    if (getRainRateCountdown() > 0) {
        obj_state["X_" + indent + "_Countndown,_szro"] = getRainRateCountdown();
    }
    obj_state[String((temp_prove ? (temp_safe ? "🟢" : "🔴") : "⚫")) + F("_Temperature,_°Czro")] = String(temperature, 1);
    obj_state[String((humi_prove ? (humi_safe ? "🟢" : "🔴") : "⚫")) + F("_Humidity,_%zro")] = String(humidity, 0);
    obj_state[indent + "_Dew_Point,_°Czro"] = String(dewpoint, 1);
    obj_state[String((dewdelta_prove ? (dewdelta_safe ? "🟢" : "🔴") : "⚫")) + F("_Dew_Point_Δ,_°Czro")] = String(dewpoint_delta, 1);
    obj_state[String((skytemp_prove ? (skytemp_safe ? "🟢" : "🔴") : "⚫")) + F("_Sky_Temp,_°Czro")] = String(tempsky, 1);
    obj_state[String((wind_prove ? (wind_safe ? "🟢" : "🔴") : "⚫")) + F("_Wind_Speed,_mzsszro")] = String(windspeed, 1);
    String safe_state_icon = "⚫";
    switch (safeunsafe_state) {
    case SafeUnsafeStatus::AWAIT_SAFE:
        safe_state_icon = "🔴🟡";
        indent = "⠀⠀⠀⠀";
        break;
    case SafeUnsafeStatus::AWAIT_UNSAFE:
        safe_state_icon = "🟢🟡";
        indent = "⠀⠀⠀⠀";
        break;
    case SafeUnsafeStatus::SAFE:
        safe_state_icon = "🟢";
        indent = "⠀⠀";
        break;
    case SafeUnsafeStatus::UNSAFE:
        safe_state_icon = "🔴";
        indent = "⠀⠀";
        break;
    }
    obj_state[safe_state_icon + F("_Is_Safezro")] = is_safe;
    if (getSafeUnsafeCountdown() > 0) {
        obj_state["Y_" + indent + "_Countndown,_szro"] = getSafeUnsafeCountdown();
    }
}
