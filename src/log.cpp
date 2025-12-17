#include "log.h"
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

void initLogPrefs() {
    logPrefs.begin("logPrefs", false);
    std::fill(std::begin(logEnabled), std::end(logEnabled), LogOn);
    std::fill(std::begin(logSlow), std::end(logSlow), 30);
    std::fill(std::begin(logTargets), std::end(logTargets), LogOn);
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

void logLine(String line, LogSource source, bool mqtt) {
    if (logEnabled[source] || source == LogConsole) {
        if (LOG_SERIAL) {
            Serial.println(line);
        }
        if (LOG_CONSOLE) {
            webSerial.println(line);
        }
        if (LOG_MQTT) {
            if (mqtt) {
                if (mqttClient)
                    mqttClient->publish(MQTT_LOG_TOPIC, mqttLogBuffer + line);
                mqttLogBuffer = "";
            }
        }
        if (LOG_LED) {
            // TODO
        }
    }
}

void logLine(String line, LogSource source) {
    logLine(line, source, true);
}

void logLinePart(String line, LogSource source, bool mqtt) {
    if (logEnabled[source] || source == LogConsole) {
        if (LOG_SERIAL) {
            Serial.print(line);
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
            // TODO
        }
    }
}
void logLinePart(String line, LogSource source) {
    logLinePart(line, source, true);
}

void logLineConsole(String line) {
    logLine(line, LogConsole, false);
}

void logLinePartConsole(String line) {
    logLinePart(line, LogConsole, false);
}

void logLineMain(String line) {
    logLine(line, LogMain);
}

void logLinePartMain(String line) {
    logLinePart(line, LogMain);
}

void logLineMeteo(String line) {
    logLine(line, LogMeteo);
}

void logLinePartMeteo(String line) {
    logLinePart(line, LogMeteo);
}

void logLineAlpaca(String line) {
    logLine(line, LogAlpaca);
}

void logLinePartAlpaca(String line) {
    logLinePart(line, LogAlpaca);
}

void logLineObscon(String line) {
    logLine(line, LogObservingConditions);
}

void logLinePartObscon(String line) {
    logLinePart(line, LogObservingConditions);
}

void logLineSafemon(String line) {
    logLine(line, LogSafetyMonitor);
}

void logLinePartSafemon(String line) {
    logLinePart(line, LogSafetyMonitor);
}

void logLineWifi(String line) {
    logLine(line, LogWifi);
}

void logLinePartWifi(String line) {
    logLinePart(line, LogWifi);
}

void logLineOta(String line) {
    logLine(line, LogOta);
}

void logLinePartOta(String line) {
    logLinePart(line, LogOta);
}

void logMessage(String msg, bool showtime) {
    if (showtime)
        logLinePartMain(logTime() + " ");
    logLineMain(msg);
}

void logMessagePart(String msg, bool showtime) {
    if (showtime)
        logLinePartMain(logTime() + " ");
    logLinePartMain(msg);
}

void logMessageConsole(String msg, bool showtime) {
    if (showtime)
        logLinePartConsole(logTime() + " ");
    logLineConsole(msg);
}

void logMessagePartConsole(String msg, bool showtime) {
    if (showtime)
        logLinePartConsole(logTime() + " ");
    logLinePartConsole(msg);
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
