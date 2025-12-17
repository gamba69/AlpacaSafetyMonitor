#pragma once

#include "main.h"
#include <Arduino.h>
#include <String.h>

#define LOG_SERIAL logTargets[TargetConsole]
#define LOG_CONSOLE logTargets[TargetConsole]
#define LOG_MQTT logTargets[TargetMQTT]
#define LOG_LED logTargets[TargetLED]

#define LOG_ENABLED_SIZE 16

extern uint8_t logEnabled[LOG_ENABLED_SIZE];
extern uint16_t logSlow[LOG_ENABLED_SIZE];
extern uint8_t logTargets[LOG_ENABLED_SIZE];

enum LogSource {
    LogMain = 0,
    LogMeteo = 1,
    LogAlpaca = 2,
    LogObservingConditions = 3,
    LogSafetyMonitor = 4,
    LogWifi = 5,
    LogOta = 6,
    LogConsole = 7
};

enum LogValues {
    LogOff = 0,
    LogOn = 1,
    LogSlow = 2
};

enum LogTarget {
    TargetSerial = 0,
    TargetConsole = 1,
    TargetMQTT = 2,
    TargetLED = 3
};

String logTime();

void initLogPrefs();
void loadLogPrefs();
void saveLogPrefs();

void logLine(String line, LogSource source);
void logLinePart(String line, LogSource source);

void logLineConsole(String line);
void logLinePartConsole(String line);

void logLineMain(String line);
void logLinePartMain(String line);

void logLineMeteo(String line);
void logLinePartMeteo(String line);

void logLineAlpaca(String line);
void logLinePartAlpaca(String line);

void logLineObscon(String line);
void logLinePartObscon(String line);

void logLineSafemon(String line);
void logLinePartSafemon(String line);

void logLineWifi(String line);
void logLinePartWifi(String line);

void logLineOta(String line);
void logLinePartOta(String line);

void logMessage(String msg, bool showtime = true);
void logMessagePart(String msg, bool showtime = false);

void logMessageConsole(String msg, bool showtime = true);
void logMessagePartConsole(String msg, bool showtime = false);

void logMqttStatus(String special = "");
