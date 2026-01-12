#pragma once

#include <Arduino.h>
#include <String.h>
#include "main.h"

#define LOG_SERIAL logTargets[LogTarget::Serial]
#define LOG_CONSOLE logTargets[LogTarget::Console]
#define LOG_MQTT logTargets[LogTarget::MQTT]
#define LOG_LED logTargets[LogTarget::LED]

#define LOG_ENABLED_SIZE 16

extern uint8_t logEnabled[LOG_ENABLED_SIZE];
extern uint16_t logSlow[LOG_ENABLED_SIZE];
extern uint8_t logTargets[LOG_ENABLED_SIZE];

class LogSource {
  public:
    static const int Main = 0;
    static const int Meteo = 1;
    static const int Alpaca = 2;
    static const int ObsCon = 3;
    static const int SafeMon = 4;
    static const int Wifi = 5;
    static const int Ota = 6;
    static const int Console = 7;
    static const int Tech = 8;
};

class Log {
  public:
    static const int Off = 0;
    static const int On = 1;
    static const int Slow = 2;
};

class LogTarget {
  public:
    static const int Serial = 0;
    static const int Console = 1;
    static const int MQTT = 2;
    static const int LED = 3;
};

String logTime();

void initLogPrefs();
void loadLogPrefs();
void saveLogPrefs();

void logLine(String line, const int source);
void logLinePart(String line, const int source);

void logMessage(String msg, bool showtime = true);
void logMessagePart(String msg, bool showtime = false);

void logConsoleMessage(String msg, bool showtime = true);
void logConsoleMessagePart(String msg, bool showtime = false);

void logTechMessage(String msg, bool showtime = true);
void logTechMessagePart(String msg, bool showtime = false);

void logMqttStatus(String special = "");
