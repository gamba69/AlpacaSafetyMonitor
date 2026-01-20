#pragma once

#include <Arduino.h>
#include <String.h>

#define CAL_DATA_SIZE 32

struct CalCoefficient {
    float a;
    float b;
    CalCoefficient() {
        a = 1;
        b = 0;
    }
    CalCoefficient(float _a, float _b) {
        a = _a;
        b = _b;
    }
};

extern CalCoefficient calData[CAL_DATA_SIZE];

void initCalPrefs();
void loadCalPrefs();
void saveCalPrefs();