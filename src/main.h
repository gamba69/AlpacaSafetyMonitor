#pragma once

#include <AlpacaServer.h>
#include <Arduino.h>
#include <ESPNtpClient.h>
#include <MycilaWebSerial.h>
#include <PicoMQTT.h>
#include <RTClib.h>
#include <WiFi.h>
#include <esp_task_wdt.h>
#include <otawebupdater.h>
#include <wifimanager.h>
#include "config.h"
#include "meteo.h"
#include "observingconditions.h"
#include "safetymonitor.h"
#include "log.h"

extern WebSerial webSerial;
extern PicoMQTT::Client *mqttClient;
extern WIFIMANAGER WifiManager;
extern OTAWEBUPDATER OtaWebUpdater;

void setup_wifi();

