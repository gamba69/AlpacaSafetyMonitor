#include "log.h"
#include "main.h"
#include <Arduino.h>
#include <Preferences.h>

String logTime() {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // gmtime_r(&now, &timeinfo);
    char strftime_buf[64]; // Ensure buffer is large enough
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(strftime_buf);
}

Preferences logPrefs;

uint8_t logEnabled[LOG_ENABLED_SIZE];
uint16_t logSlow[LOG_ENABLED_SIZE];
uint8_t logTargets[LOG_ENABLED_SIZE];

SemaphoreHandle_t serialMutex;

void initLogPrefs() {
    serialMutex = xSemaphoreCreateMutex();
    logPrefs.begin("logPrefs", false);
    std::fill(std::begin(logEnabled), std::end(logEnabled), Log::On);
    std::fill(std::begin(logSlow), std::end(logSlow), 30);
    std::fill(std::begin(logTargets), std::end(logTargets), Log::On);
    loadLogPrefs();
}

void loadLogPrefs() {
    if (logPrefs.isKey("enabled")) {
        logPrefs.getBytes("enabled", logEnabled, sizeof(logEnabled));
    }
    if (logPrefs.isKey("slow")) {
        logPrefs.getBytes("slow", logSlow, sizeof(logSlow));
    }
    if (logPrefs.isKey("targets")) {
        logPrefs.getBytes("targets", logTargets, sizeof(logTargets));
    }
}

void saveLogPrefs() {
    logPrefs.putBytes("enabled", logEnabled, sizeof(logEnabled));
    logPrefs.putBytes("slow", logSlow, sizeof(logSlow));
    logPrefs.putBytes("targets", logTargets, sizeof(logTargets));
}

String mqttLogBuffer = "";

void logLed(String line) {
    static bool usual = true;
    if (line.indexOf("[OTA] Begin firmware upgrade") != -1) {
        led.Blink(25, 100).Forever();
    }
    if (line.indexOf("[WIFI][EVENT] AP mode started!") != -1) {
        usual = false;
        // led.Blink(1200, 200).Forever();
        led.Blink(50, 50).Forever();
    }
    if (line.indexOf("[WIFI][EVENT] AP mode stopped!") != -1) {
        usual = true;
        led.Off();
    }
    if (usual && line.indexOf("[METEO][DATA]") != -1) {
        led.Blink(50, 10).Repeat(1);
    }
    if (usual && line.indexOf("[WIFI][STATUS] Connected to known") != -1) {
        led.Blink(50, 100).Repeat(4);
    }
}

void logLine(String line, const int source, bool mqtt) {
    if (logEnabled[source] || source == LogSource::Console) {
        if (LOG_SERIAL) {
            xSemaphoreTake(serialMutex, portMAX_DELAY);
            Serial.println(line);
            xSemaphoreGive(serialMutex);
        }
        if (LOG_CONSOLE) {
            webSerial.println(line);
        }
        if (LOG_MQTT) {
            if (mqtt) {
                if (mqttClient) {
                    mqttClient->publish(MQTT_LOG_TOPIC, mqttLogBuffer + line);
                }
                mqttLogBuffer = "";
            }
        }
        if (LOG_LED) {
            logLed(line);
        }
    }
}

void logLine(String line, const int source) {
    logLine(line, source, true);
}

void logLinePart(String line, int source, bool mqtt) {
    if (logEnabled[source] || source == LogSource::Console) {
        if (LOG_SERIAL) {
            xSemaphoreTake(serialMutex, portMAX_DELAY);
            Serial.print(line);
            xSemaphoreGive(serialMutex);
        }
        if (LOG_CONSOLE) {
            webSerial.print(line);
        }
        if (LOG_MQTT) {
            if (mqtt) {
                mqttLogBuffer = mqttLogBuffer + line;
            }
        }
        if (LOG_LED) {
            logLed(line);
        }
    }
}
void logLinePart(String line, int source) {
    logLinePart(line, source, true);
}

void logMessage(String msg, bool showtime) {
    if (showtime)
        logLinePart(logTime() + " ", LogSource::Main);
    logLine(msg, LogSource::Main);
}

void logMessagePart(String msg, bool showtime) {
    if (showtime)
        logLinePart(logTime() + " ", LogSource::Main);
    logLinePart(msg, LogSource::Main);
}

void logConsoleMessage(String msg, bool showtime) {
    if (showtime)
        logLinePart(logTime() + " ", LogSource::Console, false);
    logLine(msg, LogSource::Console, false);
}

void logConsoleMessagePart(String msg, bool showtime) {
    if (showtime)
        logLinePart(logTime() + " ", LogSource::Console, false);
    logLinePart(msg, LogSource::Console, false);
}

void logTechMessage(String msg, bool showtime) {
    if (showtime)
        logLinePart(logTime() + " ", LogSource::Tech);
    logLine(msg, LogSource::Tech);
}

void logTechMessagePart(String msg, bool showtime) {
    if (showtime)
        logLinePart(logTime() + " ", LogSource::Tech);
    logLinePart(msg, LogSource::Tech);
}

void logMqttStatus(String special) {
    if (special.length() > 0) {
        logMessage(special);
        return;
    }
    String mqttServer = WifiManager.getSettings("mqtt");
    if (!mqttClient) {
        logMessage("[MQTT][STATUS] Not configured");
    } else {
        if (mqttClient->connected()) {
            logMessage("[MQTT][STATUS] Connected to: " + mqttServer);
        } else {
            logMessage("[MQTT][STATUS] Disconnected from: " + mqttServer);
        }
    }
}
