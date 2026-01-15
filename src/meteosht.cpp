#include "meteosht.h"

/*
 * Optimal heating intervals table
 *
 * Source: Sensirion Application Note
 * "Using the Integrated Heater of SHT4x in High-Humidity Environments"
 * https://sensirion.com/resource/application_note/sht4x_heater_usage
 *
 * ┌──────────────┬─────────────────┬──────────────────┬──────────┬──────────────┬────────┐
 * │ Humidity RH  │ Heating Interval│ Duration         │ Power    │ Cooldown     │ Cycles │
 * ├──────────────┼─────────────────┼──────────────────┼──────────┼──────────────┼────────┤
 * │ < 60%        │ Not required    │ -                │ -        │ -            │ 0      │
 * │ 60-75%       │ 30-60 minutes   │ 1 sec            │ LOW      │ 8-10 sec     │ 1      │
 * │ 75-85%       │ 15-30 minutes   │ 1 sec            │ MEDIUM   │ 10-12 sec    │ 1      │
 * │ 85-95%       │ 10-15 minutes   │ 1 sec            │ HIGH     │ 12-15 sec    │ 1      │
 * │ > 95%        │ 5-10 minutes    │ 1 sec × 2-3 times│ HIGH     │ 15-20 sec    │ 3      │
 * └──────────────┴─────────────────┴──────────────────┴──────────┴──────────────┴────────┘
 *
 * Heater power levels:
 *   LOW    - 20 mW  (SHT4x_MEASUREMENT_LONG_LOW_HEAT)
 *   MEDIUM - 110 mW (SHT4x_MEASUREMENT_LONG_MEDIUM_HEAT)
 *   HIGH   - 200 mW (SHT4x_MEASUREMENT_LONG_HIGH_HEAT)
 *
 * Duty cycle: 10% (heating time / total time)
 * Between cycles: duty-cycle pause (1 sec heat + 9 sec pause)
 * After all cycles: full cooldown according to table
 */

const HeatingParams SHT45AutoHeat::table[5] = {
    {0, 60, 0, 0, SHT4x_MEASUREMENT_SLOW, 0, 0},
    {60, 75, 30 * 60 * 1000, 60 * 60 * 1000, SHT4x_MEASUREMENT_LONG_LOW_HEAT, 10000, 1},
    {75, 85, 15 * 60 * 1000, 30 * 60 * 1000, SHT4x_MEASUREMENT_LONG_MEDIUM_HEAT, 12000, 1},
    {85, 95, 10 * 60 * 1000, 15 * 60 * 1000, SHT4x_MEASUREMENT_LONG_HIGH_HEAT, 15000, 1},
    {95, 100, 5 * 60 * 1000, 10 * 60 * 1000, SHT4x_MEASUREMENT_LONG_HIGH_HEAT, 20000, 3},
};

SHT45AutoHeat::SHT45AutoHeat() {
}

SHT45AutoHeat::~SHT45AutoHeat() {
    if (task) {
        vTaskDelete(task);
    }
    if (semaphore) {
        vSemaphoreDelete(semaphore);
    }
}

void SHT45AutoHeat::logMessage(String msg, bool showtime) {
    if (logLine && logLinePart) {
        if (logTime && showtime) {
            logLinePart(logTime() + " ", logSource);
        }
        logLine(msg, logSource);
    }
}

void SHT45AutoHeat::logMessagePart(String msg, bool showtime) {
    if (logLinePart) {
        if (logTime && showtime) {
            logLinePart(logTime() + " ", logSource);
        }
        logLinePart(msg, logSource);
    }
}

void SHT45AutoHeat::setLogger(const int logSrc, std::function<void(String, const int)> logLineCallback, std::function<void(String, const int)> logLinePartCallback, std::function<String()> logTimeCallback) {
    logSource = logSrc;
    logLine = logLineCallback;
    logLinePart = logLinePartCallback;
    logTime = logTimeCallback;
}

bool SHT45AutoHeat::begin() {
    sht.begin();
    if (!sht.isConnected()) {
        return false;
    }
    updateHumidity();
    semaphore = xSemaphoreCreateBinary();
    if (!semaphore) {
        return false;
    }
    xSemaphoreGive(semaphore);
    return xTaskCreate(taskWrapper, "SHTHeatingTask", 2048,
                       this, 1, &task) == pdPASS;
}

SHT45Data SHT45AutoHeat::readData() {
    SHT45Data d = {0, 0, false, 0};
    if (xSemaphoreTake(semaphore, 0) != pdTRUE) {
        d.error = -1; // heating
        logMessage("[TECH][SHT45] Heating active, skip reading!");
        return d;
    }
    sht.requestData(SHT4x_MEASUREMENT_SLOW);
    uint32_t start = millis();
    while (!sht.dataReady() && millis() - start < 20) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    if (!sht.dataReady()) {
        d.error = -2; // timeout
        xSemaphoreGive(semaphore);
        logMessage("[TECH][SHT45] Timeout error!");
        return d;
    }
    if (sht.readData(true)) {
        d.temperature = sht.getTemperature();
        d.humidity = sht.getHumidity();
        d.valid = !isnan(d.temperature) && !isnan(d.humidity);
        if (d.valid) {
            humidity = d.humidity;
        }
        d.error = sht.getError();
    } else {
        d.error = sht.getError();
    }
    xSemaphoreGive(semaphore);
    if (d.error) {
        switch (d.error) {
        case SHT4x_OK:
            logMessage("[TECH][SHT45] No error occurred (you shouldn't see this)");
            break;
        case SHT4x_ERR_WRITECMD:
            logMessage("[TECH][SHT45] Write command error!");
            break;
        case SHT4x_ERR_READBYTES:
            logMessage("[TECH][SHT45] Read bytes error!");
            break;
        case SHT4x_ERR_HEATER_OFF:
            logMessage("[TECH][SHT45] Heater off error!");
            break;
        case SHT4x_ERR_NOT_CONNECT:
            logMessage("[TECH][SHT45] Not connected error!");
            break;
        case SHT4x_ERR_CRC_TEMP:
            logMessage("[TECH][SHT45] Temperature CRC error!");
            break;
        case SHT4x_ERR_CRC_HUM:
            logMessage("[TECH][SHT45] Humidity CRC error!");
            break;
        case SHT4x_ERR_HEATER_COOLDOWN:
            logMessage("[TECH][SHT45] Heater cooldown error!");
            break;
        case SHT4x_ERR_HEATER_ON:
            logMessage("[TECH][SHT45] Heater on error!");
            break;
        case SHT4x_ERR_SERIAL_NUMBER_CRC:
            logMessage("[TECH][SHT45] Serial number CRC error!");
            break;
        case 0x8B:
            logMessage("[TECH][SHT45] Invalid address error!");
            break;
        default:
            logMessage("[TECH][SHT45] Unknown error (you shouldn't see this)");
            break;
        }
    }
    return d;
}

String SHT45AutoHeat::cmdAsString(uint8_t command) {
    switch (command) {
    case SHT4x_MEASUREMENT_SLOW:
        return "SHT4x_MEASUREMENT_SLOW";
    case SHT4x_MEASUREMENT_MEDIUM:
        return "SHT4x_MEASUREMENT_MEDIUM";
    case SHT4x_MEASUREMENT_FAST:
        return "SHT4x_MEASUREMENT_FAST";
    case SHT4x_MEASUREMENT_LONG_HIGH_HEAT:
        return "SHT4x_MEASUREMENT_LONG_HIGH_HEAT";
    case SHT4x_MEASUREMENT_SHORT_HIGH_HEAT:
        return "SHT4x_MEASUREMENT_SHORT_HIGH_HEAT";
    case SHT4x_MEASUREMENT_LONG_MEDIUM_HEAT:
        return "SHT4x_MEASUREMENT_LONG_MEDIUM_HEAT";
    case SHT4x_MEASUREMENT_SHORT_MEDIUM_HEAT:
        return "SHT4x_MEASUREMENT_SHORT_MEDIUM_HEAT";
    case SHT4x_MEASUREMENT_LONG_LOW_HEAT:
        return "SHT4x_MEASUREMENT_LONG_LOW_HEAT";
    case SHT4x_MEASUREMENT_SHORT_LOW_HEAT:
        return "SHT4x_MEASUREMENT_SHORT_LOW_HEAT";
    default:
        return "";
    }
}

void SHT45AutoHeat::taskWrapper(void *p) {
    ((SHT45AutoHeat *)p)->heatingTask();
}

void SHT45AutoHeat::heatingTask() {
    while (true) {
        uint32_t now = millis();
        if (now < nextAllowed) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        const HeatingParams *p = getParams(humidity);
        if (!p) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        uint32_t interval = p->intervalMax -
                            (uint32_t)((p->intervalMax - p->intervalMin) *
                                       (humidity - p->humMin) / (p->humMax - p->humMin));
        if (interval == 0) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        if (now - lastHeat >= interval) {
            logMessage("[TECH][SHT45] Heating creep on humidity " + String(humidity, 0) + "%, after " + String(interval / (60 * 1000)) + "m, " + cmdAsString(p->cmd) + ", x" + String(p->cycles) + ", cooldown " + String(p->cooldown / 1000, 0) + "s");
            vTaskDelay(pdMS_TO_TICKS(500));
            xSemaphoreTake(semaphore, portMAX_DELAY);
            for (uint8_t i = 0; i < p->cycles; i++) {
                doHeat(p->cmd);
                if (i < p->cycles - 1) {
                    uint32_t dutyCycleDelay = getHeatDuration(p->cmd) * 9;
                    vTaskDelay(pdMS_TO_TICKS(dutyCycleDelay));
                }
            }
            lastHeat = millis();
            nextAllowed = millis() + p->cooldown;
            vTaskDelay(pdMS_TO_TICKS(p->cooldown));
            updateHumidity();
            xSemaphoreGive(semaphore);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void SHT45AutoHeat::doHeat(uint8_t cmd) {
    sht.requestData(cmd);
    uint32_t timeout = getHeatDuration(cmd) + 200;
    uint32_t start = millis();
    while (!sht.dataReady() && millis() - start < timeout) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    sht.readData(true);
}

void SHT45AutoHeat::updateHumidity() {
    sht.requestData(SHT4x_MEASUREMENT_SLOW);
    uint32_t start = millis();
    while (!sht.dataReady() && millis() - start < 20) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    if (sht.readData(true)) {
        float h = sht.getHumidity();
        if (!isnan(h))
            humidity = h;
    }
}

const HeatingParams *SHT45AutoHeat::getParams(float h) {
    for (int i = 0; i < 5; i++) {
        if (h >= table[i].humMin && h < table[i].humMax) {
            return &table[i];
        }
    }
    return NULL;
}

uint32_t SHT45AutoHeat::getHeatDuration(uint8_t cmd) {
    switch (cmd) {
    case SHT4x_MEASUREMENT_LONG_HIGH_HEAT:
    case SHT4x_MEASUREMENT_LONG_MEDIUM_HEAT:
    case SHT4x_MEASUREMENT_LONG_LOW_HEAT:
        return 1000;
    case SHT4x_MEASUREMENT_SHORT_HIGH_HEAT:
    case SHT4x_MEASUREMENT_SHORT_MEDIUM_HEAT:
    case SHT4x_MEASUREMENT_SHORT_LOW_HEAT:
        return 100;
    default:
        return 0;
    }
}