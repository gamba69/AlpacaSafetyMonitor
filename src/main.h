#pragma once
#include "config.h"
#include <Arduino.h>

#include <WiFi.h>

#include <RTClib.h>

#include "AlpacaServer.h"
#include "meteo.h"
#include "observingconditions.h"
#include "safetymonitor.h"

// weather sensors loop delay
static unsigned long measureDelay = 3000; // Sensors read cycle in ms. Always greater than 3000
static unsigned long lastTimeRan;

// module setup
void setup_wifi();

Meteo meteo("AlpacaESP32");
