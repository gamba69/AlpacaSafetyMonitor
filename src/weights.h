#pragma once

#include "main.h"
#include <Arduino.h>
#include <String.h>

#define T_WEIGHT_BMP280 thWeights[tWeightBmp280]
#define T_WEIGHT_AHT20 thWeights[tWeightAht20]
#define T_WEIGHT_SHT45 thWeights[tWeightSht45]

#define T_NORM_WEIGHT_BMP280 thWeights[tNormWeightBmp280]
#define T_NORM_WEIGHT_AHT20 thWeights[tNormWeightAht20]
#define T_NORM_WEIGHT_SHT45 thWeights[tNormWeightSht45]

#define H_WEIGHT_AHT20 thWeights[hWeightAht20]
#define H_WEIGHT_SHT45 thWeights[hWeightSht45]

#define H_NORM_WEIGHT_AHT20 thWeights[hNormWeightAht20]
#define H_NORM_WEIGHT_SHT45 thWeights[hNormWeightSht45]

#define TH_WEIGHTS_SIZE 16

extern float thWeights[TH_WEIGHTS_SIZE];

enum ThWeights {

    tWeightBmp280 = 0,
    tWeightAht20 = 1,
    tWeightSht45 = 2,

    tNormWeightBmp280 = 4,
    tNormWeightAht20 = 5,
    tNormWeightSht45 = 6,

    hWeightAht20 = 8,
    hWeightSht45 = 9,

    hNormWeightAht20 = 12,
    hNormWeightSht45 = 13,
};

void normThWeightsPrefs();
void initThWeightsPrefs();
void loadThWeightsPrefs();
void saveThWeightsPrefs();
