#include "hardware.h"
#include "config.h"
#include <Arduino.h>
#include <Preferences.h>

Preferences hwPrefs;

bool hwEnabled[HW_ENABLED_SIZE];

void calcHwPrefs() {
    hwEnabled[ocRainRate] = hwEnabled[hwUicpal] || hwEnabled[hwRg15];
    hwEnabled[ocTemperature] = hwEnabled[hwBmp280] || hwEnabled[hwAht20];
    hwEnabled[ocHumidity] = hwEnabled[hwAht20];
    hwEnabled[ocDewPoint] = hwEnabled[ocTemperature] && hwEnabled[ocHumidity];
    hwEnabled[ocPressure] = hwEnabled[hwBmp280];
    hwEnabled[ocSkyTemp] = hwEnabled[hwMlx90614];
    hwEnabled[ocCloudCover] = hwEnabled[hwMlx90614];
    hwEnabled[ocFwhm] = hwEnabled[hwMlx90614];
    hwEnabled[ocSkyBrightness] = hwEnabled[hwTsl2591];
    hwEnabled[ocSkyQuality] = hwEnabled[hwTsl2591];
    hwEnabled[ocWindDirection] = false;
    hwEnabled[ocWindSpeed] = hwEnabled[hwAnemo4403];
    hwEnabled[ocWindGust] = hwEnabled[hwAnemo4403];

    hwEnabled[smRainRate] = hwEnabled[hwUicpal] || hwEnabled[hwRg15];
    hwEnabled[smTemperature] = hwEnabled[hwBmp280] || hwEnabled[hwAht20];
    hwEnabled[smHumidity] = hwEnabled[hwAht20];
    hwEnabled[smDewPoint] = hwEnabled[smTemperature] && hwEnabled[smHumidity];
    hwEnabled[smSkyTemp] = hwEnabled[hwMlx90614];
    hwEnabled[smWindSpeed] = hwEnabled[hwAnemo4403];
}

void initHwPrefs() {
    hwPrefs.begin("hwPrefs", false);
    // Default values for current firmware
    std::fill(std::begin(hwEnabled), std::end(hwEnabled), false);
    hwEnabled[hwDs3231] = HARDWARE_DS3231;
    hwEnabled[hwBmp280] = HARDWARE_BMP280;
    hwEnabled[hwAht20] = HARDWARE_AHT20;
    hwEnabled[hwMlx90614] = HARDWARE_MLX90614;
    hwEnabled[hwTsl2591] = HARDWARE_TSL2591;
    hwEnabled[hwUicpal] = HARDWARE_UICPAL;
    hwEnabled[hwRg15] = HARDWARE_RG15;
    hwEnabled[alpacaOc] = HARDWARE_ALPACA_OC;
    hwEnabled[alpacaSm] = HARDWARE_ALPACA_SM;
    calcHwPrefs();
    loadHwPrefs();
}

void loadHwPrefs() {
    if (hwPrefs.isKey("hardware")) {
        hwPrefs.getBytes("hardware", hwEnabled, sizeof(hwEnabled));
        calcHwPrefs();
    }
}

void saveHwPrefs() {
    calcHwPrefs();
    hwPrefs.putBytes("hardware", hwEnabled, sizeof(hwEnabled));
}
