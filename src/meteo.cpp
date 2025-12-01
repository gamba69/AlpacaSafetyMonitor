#include "meteo.h"
#include "hardware.h"

#define sgn(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))

Adafruit_BMP280 bmp;
Adafruit_AHTX0 aht;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_TSL2591 tsl;

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
    xDevicesGroup = xEventGroupCreate();
    Wire.end();
    Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.begin();
    if (HARDWARE_UICPAL) {
        pinMode(RAIN_SENSOR_PIN, INPUT_PULLDOWN);
        if (digitalRead(RAIN_SENSOR_PIN)) {
            rain_rate = 1;
        } else {
            rain_rate = 0;
        }
        xTaskCreate(
            Meteo::updateUicpalWrapper,
            "updateUicpal",
            2048,
            this,
            1,
            &updateUicpalHandle);
    }
    if (HARDWARE_BMP280) {
        bmp.begin(I2C_BMP_ADDR);
        xTaskCreate(
            Meteo::updateBmp280Wrapper,
            "updateBmp280",
            2048,
            this,
            1,
            &updateBmp280Handle);
    }
    if (HARDWARE_AHT20) {
        aht.begin(&Wire, 0, I2C_AHT_ADDR);
        xTaskCreate(
            Meteo::updateAht20Wrapper,
            "updateAht20",
            2048,
            this,
            1,
            &updateAht20Handle);
    }
    if (HARDWARE_MLX90614) {
        mlx.begin(I2C_MLX_ADDR);
        xTaskCreate(
            Meteo::updateMlx90614Wrapper,
            "updateMlx80614",
            2048,
            this,
            1,
            &updateMlx90614Handle);
    }

    // xTaskCreate(
    //     Meteo::updateTsl2591Wrapper,
    //     "updateTsl2591",
    //     2048,
    //     this,
    //     1,
    //     &updateTsl2591Handle);
}

float Meteo::tsky_calc(float ts, float ta) {
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

float Meteo::cb_avg_calc() {
    int sum = 0;
    for (int i = 0; i < CB_SIZE; i++)
        sum += cb[i];
    return ((float)sum) / CB_SIZE;
}

float Meteo::cb_rms_calc() {
    int sum = 0;
    for (int i = 0; i < CB_SIZE; i++)
        sum += cb[i] * cb[i];
    return sqrt(sum / CB_SIZE);
}

void Meteo::cb_add(float value) {
    cb[cb_index] = value;
    cb_avg = cb_avg_calc();
    cb_rms = cb_rms_calc();
    cb_noise[cb_index] = abs(value) - cb_rms;
    cb_index++;
    if (cb_index == CB_SIZE)
        cb_index = 0;
}

float Meteo::cb_noise_db_calc() {
    float n = 0;
    for (int i = 0; i < CB_SIZE; i++) {
        n += cb_noise[i] * cb_noise[i];
    }
    if (n == 0)
        return 0;
    return (10 * log10(n));
}

float Meteo::cb_snr_calc() {
    float s, n = 0;
    for (int i = 0; i < CB_SIZE; i++) {
        s += cb[i] * cb[i];
        n += cb_noise[i] * cb_noise[i];
    }
    if (n == 0)
        return 0;
    return (10 * log10(s / n));
}

void Meteo::updateUicpal() {
    EventBits_t xBits;
    static unsigned long last_update = 0;
    static bool force_update = true;
    while (true) {
        if (force_update || millis() - last_update > METEO_MEASURE_DELAY) {
            if (digitalRead(RAIN_SENSOR_PIN)) {
                rain_rate = 1;
            } else {
                rain_rate = 0;
            }
            last_update = millis();
            force_update = false;
        }
        xBits = xEventGroupWaitBits(
            xDevicesGroup,
            UICPAL_KICK,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(METEO_TASK_DELAY));
        if ((xBits & UICPAL_KICK) != 0) {
            force_update = true;
        }
    }
}

void Meteo::updateBmp280() {
    EventBits_t xBits;
    static unsigned long last_update = 0;
    static bool force_update = true;
    while (true) {
        if (force_update || millis() - last_update > METEO_MEASURE_DELAY) {
            bmp_temperature = bmp.readTemperature();
            bmp_pressure = bmp.readPressure() / 100.0F;
            last_update = millis();
            force_update = false;
        }
        xBits = xEventGroupWaitBits(
            xDevicesGroup,
            BMP280_KICK,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(METEO_TASK_DELAY));
        if ((xBits & BMP280_KICK) != 0) {
            force_update = true;
        }
    }
}

void Meteo::updateAht20() {
    EventBits_t xBits;
    static unsigned long last_update = 0;
    static bool force_update = true;
    while (true) {
        if (force_update || millis() - last_update > METEO_MEASURE_DELAY) {
            sensors_event_t aht_sensor_humidity, aht_sensor_temp;
            aht.getEvent(&aht_sensor_humidity, &aht_sensor_temp);
            aht_temperature = aht_sensor_temp.temperature;
            aht_humidity = aht_sensor_humidity.relative_humidity;
            last_update = millis();
            force_update = false;
        }
        xBits = xEventGroupWaitBits(
            xDevicesGroup,
            AHT20_KICK,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(METEO_TASK_DELAY));
        if ((xBits & AHT20_KICK) != 0) {
            force_update = true;
        }
    }
}

void Meteo::updateMlx90614() {
    EventBits_t xBits;
    static unsigned long last_update = 0;
    static bool force_update = true;
    while (true) {
        if (force_update || millis() - last_update > METEO_MEASURE_DELAY) {
            mlx_tempamb = mlx.readAmbientTempC();
            mlx_tempobj = mlx.readObjectTempC();
            last_update = millis();
            force_update = false;
        }
        xBits = xEventGroupWaitBits(
            xDevicesGroup,
            MLX90614_KICK,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(METEO_TASK_DELAY));
        if ((xBits & MLX90614_KICK) != 0) {
            force_update = true;
        }
    }
}

void Meteo::updateTsl2591() {
    while (true) {
        // Serial.print("updateTsl2591 task running on core ");
        // Serial.println(xPortGetCoreID());
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void Meteo::update(bool force) {
    String message = "[METEO][DATA]";

    if (force) {
        EventBits_t xDone;
        EventBits_t xKick;
        if (HARDWARE_UICPAL) {
            xDone |= UICPAL_DONE;
            xKick |= UICPAL_KICK;
        }
        if (HARDWARE_BMP280) {
            xDone |= BMP280_DONE;
            xKick |= BMP280_KICK;
        }
        if (HARDWARE_AHT20) {
            xDone |= AHT20_DONE;
            xKick |= AHT20_KICK;
        }
        if (HARDWARE_MLX90614) {
            xDone |= MLX90614_DONE;
            xKick |= MLX90614_KICK;
        }
        xEventGroupClearBits(xDevicesGroup, xDone);
        xEventGroupSetBits(xDevicesGroup, xKick);
        xEventGroupWaitBits(xDevicesGroup, xDone, pdFALSE, pdTRUE, pdMS_TO_TICKS(METEO_FORCE_DELAY));
    }

    if (HARDWARE_UICPAL) {
        message += " RR:" + String(rain_rate, 1);
    } else {
        rain_rate = 0;
        message += " RR:n/a";
    }

    if (HARDWARE_BMP280) {
        message += " TB:" + String(bmp_temperature, 1);
        message += " PB:" + String(bmp_pressure, 0);
    } else {
        bmp_temperature = 0;
        bmp_pressure = 0;
        message += " TB:n/a PB:n/a";
    }

    if (HARDWARE_AHT20) {
        message += " TA:" + String(aht_temperature, 1);
        message += " HA:" + String(aht_humidity, 0);
    } else {
        aht_temperature = 0;
        aht_humidity = 0;
        message += " TA:n/a HA:n/a";
    }

    if (HARDWARE_BMP280 && HARDWARE_AHT20) {
        amb_temperature = (bmp_temperature + aht_temperature) / 2;
    } else if (HARDWARE_BMP280 && !HARDWARE_AHT20) {
        amb_temperature = bmp_temperature;
    } else if (!HARDWARE_BMP280 && HARDWARE_AHT20) {
        amb_temperature = aht_temperature;
    } else {
        amb_temperature = 0;
    }

    if (HARDWARE_AHT20) {
        dew_point = amb_temperature - (100 - aht_humidity) / 5.;
        message += " DP:" + String(dew_point, 1);
    } else {
        dew_point = 0;
        message += " DP:n/a";
    }

    if (HARDWARE_MLX90614) {
        message += " MA:" + String(mlx_tempamb, 1);
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

    if (logEnabled[LogMeteo] == LogOn || (logEnabled[LogMeteo] == LogSlow && millis() - last_message > logSlow[LogMeteo] * 1000)) {
        logMessage(message);
        last_message = millis();
    }
}