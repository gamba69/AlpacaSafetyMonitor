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
    {60, 75, 30 * 60 * 1000, 60 * 60 * 1000, SHT4x_MEASUREMENT_LONG_LOW_HEAT, 9000, 1},
    {75, 85, 15 * 60 * 1000, 30 * 60 * 1000, SHT4x_MEASUREMENT_LONG_MEDIUM_HEAT, 11000, 1},
    {85, 95, 10 * 60 * 1000, 15 * 60 * 1000, SHT4x_MEASUREMENT_LONG_HIGH_HEAT, 13500, 1},
    {95, 100, 5 * 60 * 1000, 10 * 60 * 1000, SHT4x_MEASUREMENT_LONG_HIGH_HEAT, 17500, 3},
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
                       this, 2, &task) == pdPASS;
}

SHT45Data SHT45AutoHeat::readData() {
    SHT45Data d = {0, 0, false, 0};
    if (xSemaphoreTake(semaphore, 0) != pdTRUE) {
        d.error = -1; // heating
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
    return d;
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
        if (now - lastHeat >= interval) {
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