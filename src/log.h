#pragma once
#include <Arduino.h>
#include <String.h>
#include "main.h"

#define LOG_ENABLED_SIZE 32

extern bool logEnabled[LOG_ENABLED_SIZE];

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
void logLinePartAlpaca(String line) ;
void logLineOC(String line);
void logLinePartOC(String line);
void logLineSM(String line);
void logLinePartSM(String line);
void logLineWifi(String line);
void logLinePartWifi(String line);
void logLineOta(String line);
void logLinePartOta(String line);
void logMessage(String msg, bool showtime = true);
void logMessagePart(String msg, bool showtime = false);
void logMessageConsole(String msg, bool showtime = true);
void logMessagePartConsole(String msg, bool showtime = false);
void logMqttStatus(String special = "");
