#pragma once
#include "config.h"
#include <Arduino.h>

#include <WiFi.h>

#include <RTClib.h>

#include "AlpacaServer.h"
#include "meteo.h"
#include "observingconditions.h"
#include "safetymonitor.h"

// module setup
void setup_wifi();
