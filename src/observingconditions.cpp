#include "observingconditions.h"
#include "meteo.h"

// cannot call member functions directly from interrupt, so need these helpers for up to 1 ObservingConditions
uint8_t ObservingConditions::_n_observingconditionss = 0;
ObservingConditions *ObservingConditions::_observingconditions_array[4] = {nullptr, nullptr, nullptr, nullptr};

bool ObservingConditions::begin() {

    // done
    _observingconditions_array[_observingconditions_index] = this;
    return true;
}

void ObservingConditions::update(Meteo meteo, unsigned long measureDelay) {
    //  update meteo
    temperature = (meteo.bmp_temperature + meteo.aht_temperature) / 2;
    humidity = meteo.aht_humidity;
    pressure = meteo.bmp_pressure;
    rainrate = meteo.rainrate;
    dewpoint = meteo.dewpoint;
    tempsky = meteo.tempsky;
    noise_db = meteo.noise_db;
    cloudcover = meteo.cloudcover;
    skyquality = meteo.skyquality;
    skybrightness = meteo.skybrightness;
};

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

    JsonObject obj_state = root[F("State")].to<JsonObject>();
    obj_state[F("Sensors_Descriptionzro")] = sensordescription;
    obj_state[F("Rain_Rate,_mm/hzro")] = String(rainrate, 1);
    obj_state[F("Temperature,_°Czro")] = String(temperature, 1);
    obj_state[F("Humidity,_zpzro")] = String(humidity, 0);
    obj_state[F("Dewpoint,_°Czro")] = String(dewpoint, 1);
    obj_state[F("Pressure,_hPazro")] = String(pressure, 0);
    obj_state[F("Sky_Temp,_°Czro")] = String(tempsky, 1);
    obj_state[F("Cloud_Cover,_zpzro")] = String(cloudcover, 0);
    // not exactly seeing (fwhm)
    obj_state[F("Turbulence,_dBzro")] = String(noise_db, 1);
    // obj_state[F("Sky Quality")] = skyquality;
    // obj_state[F("Sky Brightness")] = skybrightness;
}
