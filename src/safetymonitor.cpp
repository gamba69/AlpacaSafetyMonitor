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

bool SafetyMonitor::begin() {
    _safetymonitor_array[_safetymonitor_index] = this;
    return true;
}

void SafetyMonitor::update(Meteo meteo, unsigned long measureDelay) {
    //  update meteo
    temperature = (meteo.bmp_temperature + meteo.aht_temperature) / 2;
    humidity = meteo.aht_humidity;
    dewpoint = meteo.dewpoint;
    dewpoint_delta = (temperature - dewpoint > 0 ? temperature - dewpoint : 0);
    tempsky = meteo.tempsky;
    // windspeed = meteo.windspeed;
    if (!rain_init) {
        rainrate = meteo.rainrate;
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
    if (meteo.rainrate > 0) {
        rainrate_curr = meteo.rainrate;
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
    if (millis() - rainrate_occur >= rainrate_dry_delay && rainrate_state == RainRateState::AWAIT_WET) {
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

    _issafe = rain_safe && temp_safe && humi_safe && dewdelta_safe && skytemp_safe && wind_safe;

    // TODO Safe-Unsafe delays
};

void SafetyMonitor::aReadJson(JsonObject &root) {
    AlpacaSafetyMonitor::aReadJson(root);
    if (JsonObject obj_config = root[F("Configuration")]) {
        // Rain
        if (obj_config[F("A_Rain_Ratecomma_prove")].as<String>() == String("true"))
            rain_prove = true;
        else
            rain_prove = false;
        rainrate_wet_delay = obj_config[F("B___dash_wet_delaycomma_s")] | rainrate_wet_delay;
        rainrate_dry_delay = obj_config[F("B___dash_dry_delaycomma_s")] | rainrate_dry_delay;
        // Temp
        if (obj_config[F("C_Temperaturecomma_prove")].as<String>() == String("true"))
            temp_prove = true;
        else
            temp_prove = false;
        temp_lower_limit = obj_config[F("D___dash_lower_limitcomma_degC")] | temp_lower_limit;
        temp_upper_limit = obj_config[F("E___dash_upper_limitcomma_degC")] | temp_upper_limit;
        // Humi
        if (obj_config[F("F_Humiditycomma_prove")].as<String>() == String("true"))
            humi_prove = true;
        else
            humi_prove = false;
        humi_lower_limit = obj_config[F("G___dash_lower_limitcomma_degC")] | humi_lower_limit;
        humi_upper_limit = obj_config[F("H___dash_upper_limitcomma_degC")] | humi_upper_limit;
        // DewDelta
        if (obj_config[F("I_Dew_Point_deltacomma_prove")].as<String>() == String("true"))
            dewdelta_prove = true;
        else
            dewdelta_prove = false;
        dewdelta_lower_limit = obj_config[F("J___dash_lower_limitcomma_degC")] | dewdelta_lower_limit;
        dewdelta_upper_limit = obj_config[F("K___dash_upper_limitcomma_degC")] | dewdelta_upper_limit;
        // SkyTemp
        if (obj_config[F("L_Sky_Tempcomma_prove")].as<String>() == String("true"))
            skytemp_prove = true;
        else
            skytemp_prove = false;
        skytemp_lower_limit = obj_config[F("M___dash_lower_limitcomma_degC")] | skytemp_lower_limit;
        skytemp_upper_limit = obj_config[F("N___dash_upper_limitcomma_degC")] | skytemp_upper_limit;
        // WindSpeed
        if (obj_config[F("O_Wind_Speedcomma_prove")].as<String>() == String("true"))
            wind_prove = true;
        else
            wind_prove = false;
        wind_lower_limit = obj_config[F("P___dash_lower_limitcomma_degC")] | wind_lower_limit;
        wind_upper_limit = obj_config[F("Q___dash_upper_limitcomma_degC")] | wind_upper_limit;
    }
    // Manual for testing?
    // status_roof = root[F("State")][F("Safety Monitor Status")] | status_roof;
}

void SafetyMonitor::aWriteJson(JsonObject &root) {
    AlpacaSafetyMonitor::aWriteJson(root);
    // Configuration
    JsonObject obj_config = root[F("Configuration")].to<JsonObject>();
    obj_config[F("A_Rain_Ratecomma_prove")] = rain_prove;
    obj_config[F("B___dash_wet_delaycomma_s")] = rainrate_wet_delay;
    obj_config[F("B___dash_dry_delaycomma_s")] = rainrate_dry_delay;
    obj_config[F("C_Temperaturecomma_prove")] = temp_prove;
    obj_config[F("D___dash_lower_limitcomma_degC")] = temp_lower_limit;
    obj_config[F("E___dash_upper_limitcomma_degC")] = temp_upper_limit;
    obj_config[F("F_Humiditycomma_prove")] = humi_prove;
    obj_config[F("G___dash_lower_limitcomma_degC")] = humi_lower_limit;
    obj_config[F("H___dash_upper_limitcomma_degC")] = humi_upper_limit;
    obj_config[F("I_Dew_Point_deltacomma_prove")] = dewdelta_prove;
    obj_config[F("J___dash_lower_limitcomma_degC")] = dewdelta_lower_limit;
    obj_config[F("K___dash_upper_limitcomma_degC")] = dewdelta_upper_limit;
    obj_config[F("L_Sky_Tempcomma_prove")] = skytemp_prove;
    obj_config[F("M___dash_lower_limitcomma_degC")] = skytemp_lower_limit;
    obj_config[F("N___dash_upper_limitcomma_degC")] = skytemp_upper_limit;
    obj_config[F("O_Wind_Speedcomma_prove")] = wind_prove;
    obj_config[F("P___dash_lower_limitcomma_mslashs")] = wind_lower_limit;
    obj_config[F("Q___dash_upper_limitcomma_degC")] = wind_upper_limit;
    // State
    // 🟢 🟡 🔵 🔴 ⚫ "⠀"
    JsonObject obj_state = root[F("State")].to<JsonObject>();
    String rain_icon = "⚫";
    if (rain_prove) {
        switch (rainrate_state) {
        case RainRateState::AWAIT_DRY:
            rain_icon = "🟡";
            break;
        case RainRateState::AWAIT_WET:
            rain_icon = "🔵";
            break;
        case RainRateState::DRY:
            rain_icon = "🟢";
            break;
        case RainRateState::WET:
            rain_icon = "🔴";
            break;
        }
    }
    obj_state[rain_icon + F("_Rain_Rate,_mmslashh")] = String(rainrate, 1);
    if (getRainRateCountdown() > 0) {
        obj_state[F("___Countndown,_s")] = getRainRateCountdown();
    }
    obj_state[String((temp_prove ? (temp_safe ? "🟢" : "🔴") : "⚫")) + F("_Temperature,_°C")] = String(temperature, 1);
    obj_state[String((humi_prove ? (humi_safe ? "🟢" : "🔴") : "⚫")) + F("_Humidity,_%")] = String(humidity, 0);
    obj_state[F("___Dew_Point,_°C")] = String(dewpoint, 1);
    obj_state[String((dewdelta_prove ? (dewdelta_safe ? "🟢" : "🔴") : "⚫")) + F("_Dew_Point_Δ,_°C")] = String(dewpoint_delta, 1);
    obj_state[String((skytemp_prove ? (skytemp_safe ? "🟢" : "🔴") : "⚫")) + F("_Sky_Temp,_°C")] = String(tempsky, 1);
    obj_state[String((wind_prove ? (wind_safe ? "🟢" : "🔴") : "⚫")) + F("_Wind_Speed,_mslashs")] = String(windspeed, 1);
}
