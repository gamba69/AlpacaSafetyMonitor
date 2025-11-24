#pragma once

#include <Arduino.h>
#include <AlpacaServer.h>
#include <WiFi.h>
#include <RTClib.h>
#include <Adafruit_SleepyDog.h>
#include <ESPNtpClient.h>
#include <MycilaWebSerial.h>
#include <PicoMQTT.h>
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

