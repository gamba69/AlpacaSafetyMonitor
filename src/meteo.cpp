#include "meteo.h"
#include "hardware.h"

#define METEO_LOG_DELAY 30

Adafruit_BMP280 bmp;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_AHTX0 aht;

void Meteo::logMessage(String msg, bool showtime) {
    if (logLine && logLinePart) {
        if (logTime && showtime) {
            logLinePart(logTime() + " ");
        }
        logLine(msg);
    }
}

void Meteo::logMessagePart(String msg, bool showtime) {
    if (logLinePart) {
        if (logTime && showtime) {
            logLinePart(logTime() + " ");
        }
        logLinePart(msg);
    }
}

void Meteo::setLogger(std::function<void(String)> logLineCallback, std::function<void(String)> logLinePartCallback, std::function<String()> logTimeCallback) {
    logLine = logLineCallback;
    logLinePart = logLinePartCallback;
    logTime = logTimeCallback;
}

void Meteo::begin() {
    pinMode(RAIN_SENSOR_PIN, INPUT_PULLDOWN);
    Wire.end();
    Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.begin();
    mlx.begin(I2C_MLX_ADDR);
    bmp.begin(I2C_BMP_ADDR);
    aht.begin(&Wire, 0, I2C_AHT_ADDR);
    // Initialization
    if (digitalRead(RAIN_SENSOR_PIN)) {
        rain_rate = 1;
    } else {
        rain_rate = 0;
    }
    xTaskCreatePinnedToCore(
        Meteo::tsl2591TaskWrapper, // Task function
        "tsl2591Task",             // Task name
        10000,                     // Stack size (bytes)
        this,                      // Parameters
        1,                         // Priority
        &tsl2591TaskHandle,        // Task handle
        1                          // Core 1
    );
}

#define sgn(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))

float tsky_calc(float ts, float ta) {
    float t67, td = 0;
    float k[] = {0., 33., 0., 4., 100., 100., 0., 0.};
    if (abs(k[2] / 10. - ta) < 1) {
        t67 = sgn(k[6]) * sgn(ta - k[2] / 10.) * abs((k[2] / 10. - ta));
    } else {
        t67 = k[6] * sgn(ta - k[2] / 10.) * (log(abs((k[2] / 10 - ta))) / log(10.) + k[7] / 100);
    }
    td = (k[1] / 100.) * (ta - k[2] / 10.) + (k[3] / 100.) * pow(exp(k[4] / 1000. * ta), (k[5] / 100.)) + t67;
    return (ts - td);
}

float cb_avg_calc() {
    int sum = 0;
    for (int i = 0; i < CB_SIZE; i++)
        sum += cb[i];
    return ((float)sum) / CB_SIZE;
}

float cb_rms_calc() {
    int sum = 0;
    for (int i = 0; i < CB_SIZE; i++)
        sum += cb[i] * cb[i];
    return sqrt(sum / CB_SIZE);
}

void cb_add(float value) {
    cb[cb_index] = value;
    cb_avg = cb_avg_calc();
    cb_rms = cb_rms_calc();
    cb_noise[cb_index] = abs(value) - cb_rms;
    cb_index++;
    if (cb_index == CB_SIZE)
        cb_index = 0;
}

float cb_noise_db_calc() {
    float n = 0;
    for (int i = 0; i < CB_SIZE; i++) {
        n += cb_noise[i] * cb_noise[i];
    }
    if (n == 0)
        return 0;
    return (10 * log10(n));
}

float cb_snr_calc() {
    float s, n = 0;
    for (int i = 0; i < CB_SIZE; i++) {
        s += cb[i] * cb[i];
        n += cb_noise[i] * cb_noise[i];
    }
    if (n == 0)
        return 0;
    return (10 * log10(s / n));
}

void Meteo::tsl2591Task() {
    for (;;) {
        Serial.print("tsl2591Task running on core ");
        Serial.println(xPortGetCoreID());
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void Meteo::update() {
    String message = "[METEO][DATA]";

    if (hwEnabled[hwUicpal]) {
        if (digitalRead(RAIN_SENSOR_PIN)) {
            rain_rate = 1;
        } else {
            rain_rate = 0;
        }
        message += " RR:" + String(rain_rate, 1);
    } else {
        rain_rate = 0;
        message += " RR:n/a";
    }

    if (hwEnabled[hwBmp280]) {
        bmp_temperature = bmp.readTemperature();
        message += " TB:" + String(bmp_temperature, 1);
        bmp_pressure = bmp.readPressure() / 100.0F;
        message += " PB:" + String(bmp_pressure, 0);
    } else {
        bmp_temperature = 0;
        bmp_pressure = 0;
        message += " TB:n/a PB:n/a";
    }

    if (hwEnabled[hwAht20]) {
        sensors_event_t aht_sensor_humidity, aht_sensor_temp;
        aht.getEvent(&aht_sensor_humidity, &aht_sensor_temp);
        aht_temperature = aht_sensor_temp.temperature;
        message += " TA:" + String(aht_temperature, 1);
        aht_humidity = aht_sensor_humidity.relative_humidity;
        message += " HA:" + String(aht_humidity, 0);
    } else {
        aht_temperature = 0;
        aht_humidity = 0;
        message += " TA:n/a HA:n/a";
    }

    if (hwEnabled[hwBmp280] && hwEnabled[hwAht20]) {
        amb_temperature = (bmp_temperature + aht_temperature) / 2;
    } else if (hwEnabled[hwBmp280] && !hwEnabled[hwAht20]) {
        amb_temperature = bmp_temperature;
    } else if (!hwEnabled[hwBmp280] && hwEnabled[hwAht20]) {
        amb_temperature = aht_temperature;
    } else {
        amb_temperature = 0;
    }

    if (hwEnabled[hwAht20]) {
        dew_point = amb_temperature - (100 - aht_humidity) / 5.;
        message += " DP:" + String(dew_point, 1);
    } else {
        dew_point = 0;
        message += " DP:n/a";
    }

    if (hwEnabled[hwMlx90614]) {
        mlx_tempamb = mlx.readAmbientTempC();
        message += " MA:" + String(mlx_tempamb, 1);
        mlx_tempobj = mlx.readObjectTempC();
        message += " MO:" + String(mlx_tempobj, 1);
        sky_temperature = tsky_calc(mlx_tempobj, mlx_tempamb);
        message += " ST:" + String(sky_temperature);
        // add tempsky value to circular buffer and calculate
        // Turbulence (noise dB) / Seeing estimation
        cb_add(sky_temperature);
        noise_db = cb_noise_db_calc();
        message += " TR:" + String(noise_db, 1);
        cloud_cover = 100. + (sky_temperature * 6.);
        if (cloud_cover > 100.)
            cloud_cover = 100.;
        if (cloud_cover < 0.)
            cloud_cover = 0.;
        message += " CC:" + String(cloud_cover, 0);
    } else {
        mlx_tempamb = 0;
        mlx_tempobj = 0;
        sky_temperature = 0;
        noise_db = 0;
        cloud_cover = 0;
        message += " MA:n/a MO:n/a ST:n/a TR:n/a CC:n/a";
    }

    // TODO TSL2591

    // TODO ANEMO4403

    if (logEnabled[LogMeteo] == LogOn || (logEnabled[LogMeteo] == LogSlow && millis() - last_message > METEO_LOG_DELAY * 1000)) {
        logMessage(message);
        last_message = millis();
    }
}