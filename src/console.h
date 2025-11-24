#pragma once
#include <Arduino.h>

void commandHelp();
void commandLogState();
void commandReboot();

void commandLogMainOn();
void commandLogMainOff();

void commandLogMeteoOn();
void commandLogMeteoOff();

void commandLogAlpacaOn();
void commandLogAlpacaOff();

void commandLogOcOn();
void commandLogOcOff();

void commandLogSmOn();
void commandLogSmOff();

void commandLogWifiOn();
void commandLogWifiOff();

void commandLogOtaOn();
void commandLogOtaOff();

void commandLogOn();
void commandLogOff();

void initConsoleCommands();
void IRAM_ATTR processConsoleCommand(const std::string &msg);
