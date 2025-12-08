#include "meteo.h"
#include "hardware.h"
#include "helpers.h"

Adafruit_BMP280 bmp;
Adafruit_AHTX0 aht;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
MCPWMFreqCounter anm = MCPWMFreqCounter(WIND_SENSOR_PIN, 3000, 5000);

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
    if (HARDWARE_TSL2591) {
        tsl.begin();
        beginTslAGT(&tsl);
        // Long task
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
        // FreqCountESP.begin(WIND_SENSOR_PIN, WIND_SENSOR_MEASURE);
        anm.begin();
        xTaskCreate(
            Meteo::updateAnemo4403Wrapper,
            "updateAnemo4403",
            2048,
            this,
            1,
            &updateAnemo4403Handle);
    }
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

void Meteo::updateAnemo4403() {
    EventBits_t xBits;
    static unsigned long last_update = 0;
    static bool force_update = true;
    while (true) {
        if (force_update || millis() - last_update > 1000) {

            // while (!FreqCountESP.available()) {
            //     vTaskDelay(50);
            // }
            // uint32_t c = FreqCountESP.read();
            // Serial.println("C="+String(c));
            // float f =  c / (WIND_SENSOR_MEASURE / 1000.);
            // Serial.println("F="+String(f));
            // wind_speed = (f / 1.05) / 3.6;
            // TODO wind_gust via running average

            uint32_t c = anm.getCount();
            Serial.println("C = " + String(c));

            last_update = millis();
            force_update = false;
            xEventGroupSetBits(xDevicesGroup, ANEMO4403_DONE);
        }
        xBits = xEventGroupWaitBits(
            xDevicesGroup,
            ANEMO4403_KICK,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(1000));
        // pdMS_TO_TICKS(METEO_TASK_SLEEP));
        if ((xBits & ANEMO4403_KICK) != 0) {
            force_update = true;
        }
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
        if (HARDWARE_TSL2591) {
            xDone |= TSL2591_DONE;
            xKick |= TSL2591_KICK;
        }
        if (HARDWARE_ANEMO4403) {
            xDone |= ANEMO4403_DONE;
            xKick |= ANEMO4403_KICK;
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

    if (HARDWARE_TSL2591) {
        message += " SB: " + smart_round(sky_brightness);
        message += " SQ: " + String(sky_quality, 1);
    } else {
        message += " SB:n/a SQ:n/a";
    }

    if (HARDWARE_ANEMO4403) {
        message += " WS: " + String(wind_speed, 1);
        message += " WG: " + String(wind_gust, 1);
    } else {
        message += " WS:n/a WG:n/a";
    }
    message += " WD:n/a";

    // TODO RG15

    if (logEnabled[LogMeteo] == LogOn || (logEnabled[LogMeteo] == LogSlow && millis() - last_message > logSlow[LogMeteo] * 1000)) {
        logMessage(message);
        last_message = millis();
    }
}