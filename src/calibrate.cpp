#include "calibrate.h"
#include <Arduino.h>
#include <Preferences.h>

Preferences calPrefs;

CalCoefficient calData[CAL_DATA_SIZE];

void initCalPrefs() {
    calPrefs.begin("calPrefs", false);
    CalCoefficient defaultCal(1, 0);
    std::fill(std::begin(calData), std::end(calData), defaultCal);
    loadCalPrefs();
}

void loadCalPrefs() {
    if (calPrefs.isKey("calData")) {
        calPrefs.getBytes("calData", calData, sizeof(calData));
    }
}

void saveCalPrefs() {
    calPrefs.putBytes("calData", calData, sizeof(calData));
}
