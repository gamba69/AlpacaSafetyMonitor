#pragma once

#include <Arduino.h>
#include <string>

void commandHelp();

void commandReboot();

void commandLogState();

void commandLogMainOn();
void commandLogMainOff();

void commandLogMeteoOn();
void commandLogMeteoSlow();
void commandLogMeteoSlowDelay(uint16_t delay);
void commandLogMeteoOff();

void commandLogAlpacaOn();
void commandLogAlpacaOff();

void commandLogObsconOn();
void commandLogObsconSlow();
void commandLogObsconSlowDelay(uint16_t delay);
void commandLogObsconOff();

void commandLogSafemonOn();
void commandLogSafemonSlow();
void commandLogSafemonSlowDelay(uint16_t delay);
void commandLogSafemonOff();

void commandLogWifiOn();
void commandLogWifiOff();

void commandLogOtaOn();
void commandLogOtaOff();

void commandLogOn();
void commandLogOff();

void commandHardwareState();

void commandAlpacaObsconOn();
void commandAlpacaObsconOff();

void commandAlpacaSafemonOn();
void commandAlpacaSafemonOff();

void commandHwDs3231On();
void commandHwDs3231Off();

void commandHwBmp280On();
void commandHwBmp280Off();

void commandHwAht20On();
void commandHwAht20Off();

void commandHwMlx90614On();
void commandHwMlx90614Off();

void commandHwTsl2591On();
void commandHwTsl2591Off();

void commandHwAnemo4403On();
void commandHwAnemo4403Off();

void commandHwUicpalOn();
void commandHwUicpalOff();

void commandHwRg15On();
void commandHwRg15Off();

void initConsoleCommands();
void IRAM_ATTR processConsoleCommand(const std::string &msg);
