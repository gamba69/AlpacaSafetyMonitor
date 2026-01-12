#include "meteo.h"
#include "hardware.h"
#include "helpers.h"
#include "weights.h"

Adafruit_BMP280 bmp;
Adafruit_AHTX0 aht;
SHT45AutoHeat sht;
Adafruit_MLX90614 mlx;
TSL2591AutoGain tsl;
PCNTFrequencyCounter anm((gpio_num_t)WIND_SENSOR_PIN);
RG15 rg15(Serial0);

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

void Meteo::logTechMessage(String msg, bool showtime) {
    if (logLine && logLinePart) {
        if (logTime && showtime) {
            logLinePart(logTime() + " ", LogSource::Tech);
        }
        logLine(msg, LogSource::Tech);
    }
}

void Meteo::logTechMessagePart(String msg, bool showtime) {
    if (logLinePart) {
        if (logTime && showtime) {
            logLinePart(logTime() + " ", LogSource::Tech);
        }
        logLinePart(msg, LogSource::Tech);
    }
}

void Meteo::setLogger(const int logSrc, std::function<void(String, const int)> logLineCallback, std::function<void(String, const int)> logLinePartCallback, std::function<String()> logTimeCallback) {
    logSource = logSrc;
    logLine = logLineCallback;
    logLinePart = logLinePartCallback;
    logTime = logTimeCallback;
}

void Meteo::clearTslInterrupt() {
    tsl.clearInterrupt();
}

void Meteo::begin() {
    xDevicesGroup = xEventGroupCreate();
    Wire.end();
    Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.begin();
    if (HARDWARE_UICPAL) {
        pinMode(RAIN_SENSOR_PIN, INPUT_PULLDOWN);
        if (digitalRead(RAIN_SENSOR_PIN)) {
            sensors.uicpal_rate = 0.02;
        } else {
            sensors.uicpal_rate = 0;
        }
        xTaskCreate(
            Meteo::updateUicpalWrapper,
            "updateUicpal",
            2048,
            this,
            1,
            &updateUicpalHandle);
    }
    if (HARDWARE_RG15) {
        rg15.begin();
        if (rg15.poll()) {
            sensors.rg15_rate = rg15.getRainfallIntensity();
        }
        xTaskCreatePinnedToCore(
            Meteo::updateRg15Wrapper,
            "updateRg15",
            2048,
            this,
            1,
            &updateRg15Handle,
            1);
    }
    sensors.rain_rate = max(sensors.uicpal_rate, sensors.rg15_rate);

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
        // potentially long running
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
        // potentially long running
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
                sensors.uicpal_rate = 0.02;
            } else {
                sensors.uicpal_rate = 0;
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

void Meteo::updateRg15() {
    EventBits_t xBits;
    static unsigned long last_update = 0;
    static bool force_update = true;
    while (true) {
        if (force_update || millis() - last_update > METEO_MEASURE_DELAY) {
            // potentially long running
            if (rg15.poll()) {
                sensors.rg15_rate = rg15.getRainfallIntensity();
            } else {
                int error = rg15.getErrorCode();
                switch (error) {
                case 0:
                    logTechMessage("[TECH][RG15] No error occurred (you shouldn't see this)");
                    break;
                case 1:
                    logTechMessage("[TECH][RG15] Serial connection does not exist!");
                    break;
                case 2:
                    logTechMessage("[TECH][RG15] Serial connection could not write!");
                    break;
                case 3:
                    logTechMessage("[TECH][RG15] Response is invalid!");
                    break;
                case 4:
                    logTechMessage("[TECH][RG15] Response timed out!");
                    break;
                case 5:
                    logTechMessage("[TECH][RG15] Baud rate not supported!");
                    break;
                case 6:
                    logTechMessage("[TECH][RG15] Parsing failed!");
                    break;
                case 7:
                    logTechMessage("[TECH][RG15] Unit does not match!");
                    break;
                default:
                    break;
                }
            }
            last_update = millis();
            force_update = false;
            xEventGroupSetBits(xDevicesGroup, RG15_DONE);
        }
        xBits = xEventGroupWaitBits(
            xDevicesGroup,
            RG15_KICK,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(METEO_TASK_SLEEP));
        if ((xBits & RG15_KICK) != 0) {
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
            sensors.bmp_temperature = bmp.readTemperature();
            sensors.bmp_pressure = bmp.readPressure() / 100.0F;
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
            sensors.aht_temperature = aht_sensor_temp.temperature;
            sensors.aht_humidity = aht_sensor_humidity.relative_humidity;
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
            // potentially long running
            // TODO Smart with heating - simple NOT working!
            // Need to be heat, wait (stabilize?), read
            SHT45Data measure = sht.readData();
            if (measure.valid) {
                sensors.sht_temperature = measure.temperature;
                sensors.sht_humidity = measure.humidity;
            } else {
                int error = measure.error;
                switch (error) {
                case -1:
                    logTechMessage("[TECH][SHT45] Heating active");
                    break;
                case -2:
                    logTechMessage("[TECH][SHT45] Timeout");
                    break;
                case SHT4x_OK:
                    logTechMessage("[TECH][SHT45] No error occurred (you shouldn't see this)");
                    break;
                case SHT4x_ERR_WRITECMD:
                    logTechMessage("[TECH][SHT45] Write command error!");
                    break;
                case SHT4x_ERR_READBYTES:
                    logTechMessage("[TECH][SHT45] Read bytes error!");
                    break;
                case SHT4x_ERR_HEATER_OFF:
                    logTechMessage("[TECH][SHT45] Heater off error!");
                    break;
                case SHT4x_ERR_NOT_CONNECT:
                    logTechMessage("[TECH][SHT45] Not connected error!");
                    break;
                case SHT4x_ERR_CRC_TEMP:
                    logTechMessage("[TECH][SHT45] Temperature CRC error!");
                    break;
                case SHT4x_ERR_CRC_HUM:
                    logTechMessage("[TECH][SHT45] Humidity CRC error!");
                    break;
                case SHT4x_ERR_HEATER_COOLDOWN:
                    logTechMessage("[TECH][SHT45] Heater cooldown error!");
                    break;
                case SHT4x_ERR_HEATER_ON:
                    logTechMessage("[TECH][SHT45] Heater on error!");
                    break;
                case SHT4x_ERR_SERIAL_NUMBER_CRC:
                    logTechMessage("[TECH][SHT45] Serial number CRC error!");
                    break;
                case 0x8B:
                    logTechMessage("[TECH][SHT45] Invalid address error!");
                    break;
                default:
                    break;
                }
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
            double val;
            val = mlx.readAmbientTempC();
            if (!std::isnan(val)) {
                sensors.mlx_tempamb = val;
            }
            val = mlx.readObjectTempC();
            if (!std::isnan(val)) {
                sensors.mlx_tempobj = val;
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
            // potentially long running
            // TODO Interrupt, long delay (not METEO_MEASURE_DELAY!)
            TSL2591Data tslData = tsl.getData();
            sensors.sky_brightness = tsl.calculateLux(tslData);
            sensors.sky_quality = tsl.calculateSQM(tslData);
            last_update = millis();
            force_update = false;
            xEventGroupSetBits(xDevicesGroup, TSL2591_DONE);
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
            sensors.wind_speed = (f / 1.05) / 3.6;
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
        sensors.wind_gust = wind_gust_ra.getMaxInBuffer();
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
        if (HARDWARE_RG15) {
            xDone |= RG15_DONE;
            xKick |= RG15_KICK;
            // xWait |= RG15_DONE; // potentially long running
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
            // xWait |= SHT45_DONE; // potentially long running
        }
        if (HARDWARE_MLX90614) {
            xDone |= MLX90614_DONE;
            xKick |= MLX90614_KICK;
            xWait |= MLX90614_DONE;
        }
        if (HARDWARE_TSL2591) {
            xDone |= TSL2591_DONE;
            xKick |= TSL2591_KICK;
            // xWait |= TSL2591_DONE; // potentially long running
        }
        if (HARDWARE_ANEMO4403) {
            xDone |= ANEMO4403_DONE;
            xKick |= ANEMO4403_KICK;
            xWait |= ANEMO4403_DONE;
        }
        xEventGroupClearBits(xDevicesGroup, xDone);
        xEventGroupSetBits(xDevicesGroup, xKick);
        xEventGroupWaitBits(xDevicesGroup, xWait, pdFALSE, pdTRUE, pdMS_TO_TICKS(METEO_FORCE_DELAY));
    }

    if (HARDWARE_UICPAL) {
        message += " UR:" + trimmed(sensors.uicpal_rate, 2);
    } else {
        sensors.uicpal_rate = 0;
        message += " UR:n/a";
    }

    if (HARDWARE_RG15) {
        message += " RR:" + trimmed(sensors.rg15_rate, 2);
    } else {
        sensors.rg15_rate = 0;
        message += " RR:n/a";
    }

    sensors.rain_rate = max(sensors.uicpal_rate, sensors.rg15_rate);

    if (HARDWARE_BMP280) {
        message += " TB:" + trimmed(sensors.bmp_temperature, 1);
        message += " PB:" + trimmed(sensors.bmp_pressure, 0);
    } else {
        sensors.bmp_temperature = 0;
        sensors.bmp_pressure = 0;
        message += " TB:n/a PB:n/a";
    }

    if (HARDWARE_AHT20) {
        message += " TA:" + trimmed(sensors.aht_temperature, 1);
        message += " HA:" + trimmed(sensors.aht_humidity, 0);
    } else {
        sensors.aht_temperature = 0;
        sensors.aht_humidity = 0;
        message += " TA:n/a HA:n/a";
    }

    if (HARDWARE_SHT45) {
        message += " TS:" + trimmed(sensors.sht_temperature, 1);
        message += " HS:" + trimmed(sensors.sht_humidity, 0);
    } else {
        sensors.sht_temperature = 0;
        sensors.sht_humidity = 0;
        message += " TS:n/a HS:n/a";
    }

    sensors.temperature = T_NORM_WEIGHT_BMP280 * sensors.bmp_temperature + T_NORM_WEIGHT_AHT20 * sensors.aht_temperature + T_NORM_WEIGHT_SHT45 * sensors.sht_temperature;
    sensors.humidity = H_NORM_WEIGHT_AHT20 * sensors.aht_humidity + H_NORM_WEIGHT_SHT45 * sensors.sht_humidity;

    if (HARDWARE_AHT20 || HARDWARE_SHT45) {
        sensors.dew_point = sensors.temperature - (100 - sensors.humidity) / 5.;
        message += " DP:" + trimmed(sensors.dew_point, 1);
    } else {
        sensors.dew_point = 0;
        message += " DP:n/a";
    }

    if (HARDWARE_MLX90614) {
        message += " MA:" + trimmed(sensors.mlx_tempamb, 1);
        message += " MO:" + trimmed(sensors.mlx_tempobj, 1);
        sensors.sky_temperature = tsky_calc(sensors.mlx_tempobj, sensors.mlx_tempamb);
        message += " ST:" + trimmed(sensors.sky_temperature, 1);
        // add tempsky value to circular buffer and calculate
        // Turbulence (noise dB) / Seeing estimation
        cb_add(sensors.sky_temperature);
        sensors.noise_db = cb_noise_db_calc();
        message += " TR:" + trimmed(sensors.noise_db, 1);
        sensors.cloud_cover = 100. + (sensors.sky_temperature * 6.);
        if (sensors.cloud_cover > 100.)
            sensors.cloud_cover = 100.;
        if (sensors.cloud_cover < 0.)
            sensors.cloud_cover = 0.;
        message += " CC:" + trimmed(sensors.cloud_cover, 0);
    } else {
        sensors.mlx_tempamb = 0;
        sensors.mlx_tempobj = 0;
        sensors.sky_temperature = 0;
        sensors.noise_db = 0;
        sensors.cloud_cover = 0;
        message += " MA:n/a MO:n/a ST:n/a TR:n/a CC:n/a";
    }

    if (HARDWARE_TSL2591) {
        message += " SB:" + smart_round(sensors.sky_brightness);
        message += " SQ:" + trimmed(sensors.sky_quality, 1);
    } else {
        message += " SB:n/a SQ:n/a";
    }

    if (HARDWARE_ANEMO4403) {
        message += " WS:" + trimmed(sensors.wind_speed, 1);
        message += " WG:" + trimmed(sensors.wind_gust, 1);
    } else {
        message += " WS:n/a WG:n/a";
    }
    message += " WD:n/a";

    if (logEnabled[LogSource::Meteo] == Log::On || (logEnabled[LogSource::Meteo] == Log::Slow && millis() - last_message > logSlow[LogSource::Meteo] * 1000)) {
        logMessage(message);
        last_message = millis();
    }
}