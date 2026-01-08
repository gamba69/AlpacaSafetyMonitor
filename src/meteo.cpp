#include "meteo.h"
#include "hardware.h"
#include "helpers.h"
#include "weights.h"

Adafruit_BMP280 bmp;
Adafruit_AHTX0 aht;
SHT4x sht;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
PCNTFrequencyCounter anm = PCNTFrequencyCounter((gpio_num_t)WIND_SENSOR_PIN);

void Meteo::logMessage(String msg, bool showtime) {
    if (logLine && logLinePart) {
        if (logTime && showtime) {
            logLinePart(logTime() + " ", logSource);
        }
        logLine(msg, logSource);
    }
}

void Meteo::logMessagePart(String msg, bool showtime) {
    if (logLinePart) {
        if (logTime && showtime) {
            logLinePart(logTime() + " ", logSource);
        }
        logLinePart(msg, logSource);
    }
}

void Meteo::setLogger(const int logSrc, std::function<void(String, const int)> logLineCallback, std::function<void(String, const int)> logLinePartCallback, std::function<String()> logTimeCallback) {
    logSource = logSrc;
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
    // TODO RG15
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
    if (HARDWARE_SHT45) {
        sht.begin();
        // potentially slow
        xTaskCreatePinnedToCore(
            Meteo::updateSht45Wrapper,
            "updateSht45",
            2048,
            this,
            1,
            &updateSht45Handle,
            1);
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
    if (HARDWARE_TSL2591) {
        tsl.begin();
        beginTslAGT(&tsl);
        // potentially slow
        xTaskCreatePinnedToCore(
            Meteo::updateTsl2591Wrapper,
            "updateTsl2591",
            2048,
            this,
            1,
            &updateTsl2591Handle,
            1);
    }
    if (HARDWARE_ANEMO4403) {
        anm.begin();
        xTaskCreate(
            Meteo::updateAnemo4403SpeedWrapper,
            "updateAnemo4403Speed",
            2048,
            this,
            1,
            &updateAnemo4403SpeedHandle);
        xTaskCreate(
            Meteo::updateAnemo4403GustWrapper,
            "updateAnemo4403Gust",
            2048,
            this,
            1,
            &updateAnemo4403GustHandle);
    }
}

void Meteo::updateUicpal() {
    EventBits_t xBits;
    static unsigned long last_update = 0;
    static bool force_update = true;
    while (true) {
        if (force_update || millis() - last_update > METEO_MEASURE_DELAY) {
            if (digitalRead(RAIN_SENSOR_PIN)) {
                rain_rate = 0.1;
            } else {
                rain_rate = 0;
            }
            last_update = millis();
            force_update = false;
            xEventGroupSetBits(xDevicesGroup, UICPAL_DONE);
        }
        xBits = xEventGroupWaitBits(
            xDevicesGroup,
            UICPAL_KICK,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(METEO_TASK_SLEEP));
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
            xEventGroupSetBits(xDevicesGroup, BMP280_DONE);
        }
        xBits = xEventGroupWaitBits(
            xDevicesGroup,
            BMP280_KICK,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(METEO_TASK_SLEEP));
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
            xEventGroupSetBits(xDevicesGroup, AHT20_DONE);
        }
        xBits = xEventGroupWaitBits(
            xDevicesGroup,
            AHT20_KICK,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(METEO_TASK_SLEEP));
        if ((xBits & AHT20_KICK) != 0) {
            force_update = true;
        }
    }
}

void Meteo::updateSht45() {
    EventBits_t xBits;
    static unsigned long last_update = 0;
    static bool force_update = true;
    while (true) {
        if (force_update || millis() - last_update > METEO_MEASURE_DELAY) {
            // TODO Smart with heating
            if (sht.read()) {
                sht_temperature = sht.getTemperature();
                sht_humidity = sht.getHumidity();
            } else {
                // TODO print error?
            }
            last_update = millis();
            force_update = false;
            xEventGroupSetBits(xDevicesGroup, SHT45_DONE);
        }
        xBits = xEventGroupWaitBits(
            xDevicesGroup,
            SHT45_KICK,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(METEO_TASK_SLEEP));
        if ((xBits & SHT45_KICK) != 0) {
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
            float val;
            val = mlx.readAmbientTempC();
            if (val != NAN) {
                mlx_tempamb = val;
            }
            val = mlx.readObjectTempC();
            if (val != NAN) {
                mlx_tempobj = val;
            }
            last_update = millis();
            force_update = false;
            xEventGroupSetBits(xDevicesGroup, MLX90614_DONE);
        }
        xBits = xEventGroupWaitBits(
            xDevicesGroup,
            MLX90614_KICK,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(METEO_TASK_SLEEP));
        if ((xBits & MLX90614_KICK) != 0) {
            force_update = true;
        }
    }
}

void Meteo::updateTsl2591() {
    EventBits_t xBits;
    static unsigned long last_update = 0;
    static bool force_update = true;
    while (true) {
        if (force_update || millis() - last_update > METEO_MEASURE_DELAY) {
            xEventGroupSetBits(xDevicesGroup, TSL2591_DONE); // long running
            TslAutoLum agt = getTslAGT(&tsl);
            sky_brightness = calcLuxAGT(agt);
            sky_quality = calcSqmAGT(agt);
            last_update = millis();
            force_update = false;
        }
        xBits = xEventGroupWaitBits(
            xDevicesGroup,
            TSL2591_KICK,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(METEO_TASK_SLEEP));
        if ((xBits & TSL2591_KICK) != 0) {
            force_update = true;
        }
    }
}

void Meteo::updateAnemo4403Speed() {
    EventBits_t xBits;
    static unsigned long last_update = 0;
    static bool force_update = true;
    while (true) {
        if (force_update || millis() - last_update > METEO_MEASURE_DELAY) {
            // Different cycles for wind_speed (custom)
            // and wind_gust (always 3 sec then 2 minutes max)
            // 40 values max - every 3 sec on 2 minutes
            float f = anm.getFrequency(METEO_MEASURE_DELAY);
            wind_speed = (f / 1.05) / 3.6;
            last_update = millis();
            force_update = false;
            xEventGroupSetBits(xDevicesGroup, ANEMO4403_DONE);
        }
        xBits = xEventGroupWaitBits(
            xDevicesGroup,
            ANEMO4403_KICK,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(METEO_TASK_SLEEP));
        if ((xBits & ANEMO4403_KICK) != 0) {
            force_update = true;
        }
    }
}

void Meteo::updateAnemo4403Gust() {
    // May not be forced at all
    while (true) {
        // Different cycles for wind_speed (custom)
        // and wind_gust (always 3 sec then 2 minutes max)
        // 40 values max - every 3 sec on 2 minutes
        float f = anm.getFrequency(3000);
        float s = (f / 1.05) / 3.6;
        wind_gust_ra.add(s);
        wind_gust = wind_gust_ra.getMaxInBuffer();
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

String Meteo::trimmed(float v, int p) {
    String s = String(v, p);
    s.replace(" ", "");
    return s;
}

void Meteo::update(bool force) {
    String message = "[METEO][DATA]";

    if (force) {
        EventBits_t xDone;
        EventBits_t xKick;
        EventBits_t xWait;
        if (HARDWARE_UICPAL) {
            xDone |= UICPAL_DONE;
            xKick |= UICPAL_KICK;
            xWait |= UICPAL_DONE;
        }
        if (HARDWARE_BMP280) {
            xDone |= BMP280_DONE;
            xKick |= BMP280_KICK;
            xWait |= BMP280_DONE;
        }
        if (HARDWARE_AHT20) {
            xDone |= AHT20_DONE;
            xKick |= AHT20_KICK;
            xWait |= AHT20_DONE;
        }
        if (HARDWARE_SHT45) {
            xDone |= SHT45_DONE;
            xKick |= SHT45_KICK;
            // xWait |= SHT45_DONE; // potentially slow
        }
        if (HARDWARE_MLX90614) {
            xDone |= MLX90614_DONE;
            xKick |= MLX90614_KICK;
            xWait |= MLX90614_DONE;
        }
        if (HARDWARE_TSL2591) {
            xDone |= TSL2591_DONE;
            xKick |= TSL2591_KICK;
            // xWait |= TSL2591_DONE; // potentially slow
        }
        if (HARDWARE_ANEMO4403) {
            xDone |= ANEMO4403_DONE;
            xKick |= ANEMO4403_KICK;
            xWait |= ANEMO4403_DONE;
        }
        // TODO RG15
        xEventGroupClearBits(xDevicesGroup, xDone);
        xEventGroupSetBits(xDevicesGroup, xKick);
        xEventGroupWaitBits(xDevicesGroup, xWait, pdFALSE, pdTRUE, pdMS_TO_TICKS(METEO_FORCE_DELAY));
    }

    // TODO RG15
    if (HARDWARE_UICPAL) {
        message += " RR:" + trimmed(rain_rate, 1);
    } else {
        rain_rate = 0;
        message += " RR:n/a";
    }

    if (HARDWARE_BMP280) {
        message += " TB:" + trimmed(bmp_temperature, 1);
        message += " PB:" + trimmed(bmp_pressure, 0);
    } else {
        bmp_temperature = 0;
        bmp_pressure = 0;
        message += " TB:n/a PB:n/a";
    }

    if (HARDWARE_AHT20) {
        message += " TA:" + trimmed(aht_temperature, 1);
        message += " HA:" + trimmed(aht_humidity, 0);
    } else {
        aht_temperature = 0;
        aht_humidity = 0;
        message += " TA:n/a HA:n/a";
    }

    if (HARDWARE_SHT45) {
        message += " TS:" + trimmed(sht_temperature, 1);
        message += " HS:" + trimmed(sht_humidity, 0);
    } else {
        sht_temperature = 0;
        sht_humidity = 0;
        message += " TS:n/a HS:n/a";
    }

    amb_temperature = T_NORM_WEIGHT_BMP280 * bmp_temperature + T_NORM_WEIGHT_AHT20 * aht_temperature + T_NORM_WEIGHT_SHT45 * sht_temperature;
    amb_humidity = H_NORM_WEIGHT_AHT20 * aht_humidity + H_NORM_WEIGHT_SHT45 * sht_humidity;

    if (HARDWARE_AHT20 || HARDWARE_SHT45) {
        dew_point = amb_temperature - (100 - amb_humidity) / 5.;
        message += " DP:" + trimmed(dew_point, 1);
    } else {
        dew_point = 0;
        message += " DP:n/a";
    }

    if (HARDWARE_MLX90614) {
        message += " MA:" + trimmed(mlx_tempamb, 1);
        message += " MO:" + trimmed(mlx_tempobj, 1);
        sky_temperature = tsky_calc(mlx_tempobj, mlx_tempamb);
        message += " ST:" + trimmed(sky_temperature, 1);
        // add tempsky value to circular buffer and calculate
        // Turbulence (noise dB) / Seeing estimation
        cb_add(sky_temperature);
        noise_db = cb_noise_db_calc();
        message += " TR:" + trimmed(noise_db, 1);
        cloud_cover = 100. + (sky_temperature * 6.);
        if (cloud_cover > 100.)
            cloud_cover = 100.;
        if (cloud_cover < 0.)
            cloud_cover = 0.;
        message += " CC:" + trimmed(cloud_cover, 0);
    } else {
        mlx_tempamb = 0;
        mlx_tempobj = 0;
        sky_temperature = 0;
        noise_db = 0;
        cloud_cover = 0;
        message += " MA:n/a MO:n/a ST:n/a TR:n/a CC:n/a";
    }

    if (HARDWARE_TSL2591) {
        message += " SB:" + smart_round(sky_brightness);
        message += " SQ:" + trimmed(sky_quality, 1);
    } else {
        message += " SB:n/a SQ:n/a";
    }

    if (HARDWARE_ANEMO4403) {
        message += " WS:" + trimmed(wind_speed, 1);
        message += " WG:" + trimmed(wind_gust, 1);
    } else {
        message += " WS:n/a WG:n/a";
    }
    message += " WD:n/a";

    if (logEnabled[LogSource::Meteo] == Log::On || (logEnabled[LogSource::Meteo] == Log::Slow && millis() - last_message > logSlow[LogSource::Meteo] * 1000)) {
        logMessage(message);
        last_message = millis();
    }
}