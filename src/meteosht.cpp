#include "meteosht.h"

/*
 * Таблица оптимальных интервалов подогрева
 *
 * Источник: Sensirion Application Note
 * "Using the Integrated Heater of SHT4x in High-Humidity Environments"
 * https://sensirion.com/resource/application_note/sht4x_heater_usage
 *
 * ┌──────────────┬─────────────────┬──────────────────┬──────────┬──────────────┬────────┐
 * │ Влажность RH │ Интервал нагрева│ Длительность     │ Мощность │ Охлаждение   │ Циклы  │
 * ├──────────────┼─────────────────┼──────────────────┼──────────┼──────────────┼────────┤
 * │ < 60%        │ Не требуется    │ -                │ -        │ -            │ 0      │
 * │ 60-75%       │ 30-60 минут     │ 1 сек            │ LOW      │ 8-10 сек     │ 1      │
 * │ 75-85%       │ 15-30 минут     │ 1 сек            │ MEDIUM   │ 10-12 сек    │ 1      │
 * │ 85-95%       │ 10-15 минут     │ 1 сек            │ HIGH     │ 12-15 сек    │ 1      │
 * │ > 95%        │ 5-10 минут      │ 1 сек × 2-3 раза │ HIGH     │ 15-20 сек    │ 3      │
 * └──────────────┴─────────────────┴──────────────────┴──────────┴──────────────┴────────┘
 *
 * Мощности нагревателя:
 *   LOW    - 20 mW  (SHT4x_MEASUREMENT_LONG_LOW_HEAT)
 *   MEDIUM - 110 mW (SHT4x_MEASUREMENT_LONG_MEDIUM_HEAT)
 *   HIGH   - 200 mW (SHT4x_MEASUREMENT_LONG_HIGH_HEAT)
 *
 * Duty cycle: 10% (время нагрева / общее время)
 * Между циклами: duty-cycle пауза (1 сек нагрев + 9 сек пауза)
 * После всех циклов: полное охлаждение согласно таблице
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
    SHT45Data d = {0, 0, false};
    if (xSemaphoreTake(semaphore, 0) != pdTRUE) {
        return d;
    }
    sht.requestData(SHT4x_MEASUREMENT_SLOW);
    uint32_t start = millis();
    while (!sht.dataReady() && millis() - start < 20) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    if (sht.readData(true)) {
        d.temperature = sht.getTemperature();
        d.humidity = sht.getHumidity();
        d.valid = !isnan(d.temperature) && !isnan(d.humidity);
        if (d.valid) {
            humidity = d.humidity;
        }
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