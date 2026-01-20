#pragma once

#include "hardware.h"
#include <Arduino.h>

String smart_round(float);
String uptime();
void faults(int *, String *);
