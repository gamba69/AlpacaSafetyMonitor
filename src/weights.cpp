#include "weights.h"
#include "hardware.h"
#include <Arduino.h>
#include <Preferences.h>

Preferences weightsPrefs;

float thWeights[TH_WEIGHTS_SIZE];

void normThWeightsPrefs() {
    float s;
    s = T_WEIGHT_BMP280 + T_WEIGHT_AHT20 + T_WEIGHT_SHT45;
    T_NORM_WEIGHT_BMP280 = T_WEIGHT_BMP280 / s;
    T_NORM_WEIGHT_AHT20 = T_WEIGHT_AHT20 / s;
    T_NORM_WEIGHT_SHT45 = T_WEIGHT_SHT45 / s;
    s = H_WEIGHT_AHT20 + H_WEIGHT_SHT45;
    H_NORM_WEIGHT_AHT20 = H_WEIGHT_AHT20 / s;
    H_NORM_WEIGHT_SHT45 = H_WEIGHT_SHT45 / s;
}

void initThWeightsPrefs() {
    weightsPrefs.begin("weightPrefs", false);
    // Default values for current firmware
    std::fill(std::begin(thWeights), std::end(thWeights), 0);
    if (HARDWARE_SHT45) {
        T_WEIGHT_SHT45 = 1;
        H_WEIGHT_SHT45 = 1;
    } else if (HARDWARE_AHT20) {
        T_WEIGHT_AHT20 = 1;
        H_WEIGHT_AHT20 = 1;
    } else if (HARDWARE_BMP280) {
        T_WEIGHT_BMP280 = 1;
    }
    normThWeightsPrefs();
    loadThWeightsPrefs();
}

void loadThWeightsPrefs() {
    if (weightsPrefs.isKey("weights")) {
        weightsPrefs.getBytes("weights", thWeights, sizeof(thWeights));
        normThWeightsPrefs();
    }
}

void saveThWeightsPrefs() {
    normThWeightsPrefs();
    weightsPrefs.putBytes("weights", thWeights, sizeof(thWeights));
}
