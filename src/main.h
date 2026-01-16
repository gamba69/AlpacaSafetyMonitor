#pragma once

#include "config.h"
#include "log.h"
#include "meteo.h"
#include "observingconditions.h"
#include "safetymonitor.h"
#include "helpers.h"
#include <AlpacaServer.h>
#include <Arduino.h>
#include <ESPNtpClient.h>
#include <MycilaWebSerial.h>
#include <PicoMQTT.h>
#include <RTClib.h>
#include <WiFi.h>
#include <esp_task_wdt.h>
#include <jled.h>
#include <otawebupdater.h>
#include <wifimanager.h>

extern WebSerial webSerial;
extern PicoMQTT::Client *mqttClient;
extern JLed led;
extern WIFIMANAGER WifiManager;
extern OTAWEBUPDATER OtaWebUpdater;

void setup_wifi();
