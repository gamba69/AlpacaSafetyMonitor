#include "meteorg15.h"
#include <cmath>

bool RGAsync::begin(int events) {
    this->events = events;
    rg15.begin();
    xRg15Events = xEventGroupCreate();
    dataMutex = xSemaphoreCreateMutex();
    xSemaphoreGive(dataMutex);
    RGData d = getLastData();
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
        lastData = d;
        xSemaphoreGive(dataMutex);
    }
    return xTaskCreatePinnedToCore(
               taskWrapper,
               "RG15UpdatingTask",
               4096,
               this,
               1,
               &task,
               1) == pdPASS;
}

void RGAsync::setDataReadyCallback(std::function<void()> dataReadyCallback) {
    this->dataReadyCallback = dataReadyCallback;
}

void RGAsync::taskWrapper(void *p) {
    ((RGAsync *)p)->updatingTask();
}

void RGAsync::updatingTask() {
    EventBits_t xBits;
    static unsigned long lastUpdate = 0;
    static bool forced = false;
    while (true) {
        uint32_t now = millis();
        if (forced || now - lastUpdate >= METEO_MEASURE_DELAY) {
            RGData d = getLastData();
            if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
                lastData = d;
                xSemaphoreGive(dataMutex);
            }
            lastUpdate = millis();
            if (dataReadyCallback) {
                if (events & RGEvents::DATAREADY_CALLBACK) {
                    dataReadyCallback();
                }
            }
            forced = false;
        }
        xBits = xEventGroupWaitBits(
            xRg15Events,
            RG_KICK,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(METEO_TASK_SLEEP));
        if ((xBits & RG_KICK) != 0) {
            forced = true;
        }
    }
}

void RGAsync::forceUpdate() {
    xEventGroupSetBits(xRg15Events, RG_KICK);
}

void RGAsync::logMessage(String msg, bool showtime) {
    if (logLine && logLinePart) {
        if (logTime && showtime) {
            logLinePart(logTime() + " ", logSource);
        }
        logLine(msg, logSource);
    }
}

void RGAsync::logMessagePart(String msg, bool showtime) {
    if (logLinePart) {
        if (logTime && showtime) {
            logLinePart(logTime() + " ", logSource);
        }
        logLinePart(msg, logSource);
    }
}

void RGAsync::setLogger(const int logSrc, std::function<void(String, const int)> logLineCallback, std::function<void(String, const int)> logLinePartCallback, std::function<String()> logTimeCallback) {
    logSource = logSrc;
    logLine = logLineCallback;
    logLinePart = logLinePartCallback;
    logTime = logTimeCallback;
}

RGData RGAsync::getData() {
    RGData d;
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
        d = lastData;
        xSemaphoreGive(dataMutex);
    }
    return d;
}

RGData RGAsync::getLastData() {
    RGData d;
    if (rg15.poll()) {
        d.rainfallIntensity = rg15.getRainfallIntensity();
    } else {
        d.rainfallIntensity = lastData.rainfallIntensity;
        int error = rg15.getErrorCode();
        switch (error) {
        case 0:
            logMessage("[TECH][RG15] No error occurred (you shouldn't see this)");
            break;
        case 1:
            logMessage("[TECH][RG15] Serial connection does not exist!");
            break;
        case 2:
            logMessage("[TECH][RG15] Serial connection could not write!");
            break;
        case 3:
            logMessage("[TECH][RG15] Response is invalid!");
            break;
        case 4:
            logMessage("[TECH][RG15] Response timed out!");
            break;
        case 5:
            logMessage("[TECH][RG15] Baud rate not supported!");
            break;
        case 6:
            logMessage("[TECH][RG15] Parsing failed!");
            break;
        case 7:
            logMessage("[TECH][RG15] Unit does not match!");
            break;
        default:
            logMessage("[TECH][RG15] Unknown error (you shouldn't see this)");
            break;
        }
    }
    return d;
}
