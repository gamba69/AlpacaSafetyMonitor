#include "meteotsl.h"
#include "hardware.h"
#include "meteo.h"

/*
    channel_0 (full) should probably never be less than channel_1 (ir)
    under normal circumstances so we set the gain and integration
    time based only on channel_0.
    There are 24 combinations of gain and integration time for this device.
    Most are not particularly useful for general applications so to simplify,
    we choose 7 that overlap neatly for a bumpless transition between states
    that also enables access to the entire dynamic range of the device.
    Calculate the thresholds for 0.05 and 0.95 of the max range
    then pick your states.

    Gain I,ms   Max	    ATM	    Lo	    Hi	R Thresholds
       1  100 36863     100      5      95	1 1843 35020
       1  200 65535     200     10     190	  3277 62258
       1  300 65535     300     15     285	  3277 62258
       1  400 65535     400     20     380	  3277 62258
       1  500 65535     500     25     475	  3277 62258
       1  600 65535     600     30     570	2 3277 62258
      25  100 36863    2500    125    2375	  1843 35020
      25  200 65535    5000    250    4750	3 3277 62258
      25  300 65535    7500    375    7125	  3277 62258
      25  400 65535   10000    500    9500	  3277 62258
      25  500 65535   12500    625   11875	  3277 62258
      25  600 65535   15000    750   14250	  3277 62258
     428  100 36863   42800   2140   40660	4 1843 35020
     428  200 65535   85600   4280   81320	  3277 62258
     428  300 65535  128400   6420	121980	  3277 62258
     428  400 65535  171200   8560	162640	  3277 62258
     428  500 65535  214000  10700	203300	  3277 62258
     428  600 65535  256800  12840	243960	5 3277 62258
    9876  100 36863  987600  49380	938220	  1843 35020
    9876  200 65535 1975200  98760 1876440	6 3277 62258
    9876  300 65535 2962800 148140 2814660	  3277 62258
    9876  400 65535 3950400 197520 3752880	  3277 62258
    9876  500 65535 4938000 246900 4461100	  3277 62258
    9876  600 65535 5925600 296280 5629320	7 3277 62258

    Reference:
    https://www.adafruit.com/product/1980
    https://github.com/adafruit/Adafruit_TSL2591_Library
    https://cdn-learn.adafruit.com/assets/assets/000/078/658/original/TSL2591_DS000338_6-00.pdf?1564168468
*/

float tslTimeAsMillis(tsl2591IntegrationTime_t t) {
    switch (t) {
    case TSL2591_INTEGRATIONTIME_100MS:
        return 100.0F;
    case TSL2591_INTEGRATIONTIME_200MS:
        return 200.0F;
    case TSL2591_INTEGRATIONTIME_300MS:
        return 300.0F;
    case TSL2591_INTEGRATIONTIME_400MS:
        return 400.0F;
    case TSL2591_INTEGRATIONTIME_500MS:
        return 500.0F;
    case TSL2591_INTEGRATIONTIME_600MS:
        return 600.0F;
    default: // 100ms
        return 100.0F;
    }
}

float tslGainAsMulti(tsl2591Gain_t g) {
    switch (g) {
    case TSL2591_GAIN_LOW:
        return 1.0F;
    case TSL2591_GAIN_MED:
        return 25.0F;
    case TSL2591_GAIN_HIGH:
        return 428.0F;
    case TSL2591_GAIN_MAX:
        return 9876.0F;
    default:
        return 1.0F;
    }
}

void Meteo::beginTslAGT(Adafruit_TSL2591 *tsl) {
    tslAgt[0] = TslSetting(TSL2591_GAIN_LOW, TSL2591_INTEGRATIONTIME_100MS, 1843, 35020);
    tslAgt[1] = TslSetting(TSL2591_GAIN_LOW, TSL2591_INTEGRATIONTIME_600MS, 3277, 62258);
    tslAgt[2] = TslSetting(TSL2591_GAIN_MED, TSL2591_INTEGRATIONTIME_200MS, 3277, 62258);
    tslAgt[3] = TslSetting(TSL2591_GAIN_HIGH, TSL2591_INTEGRATIONTIME_100MS, 1843, 35020);
    tslAgt[4] = TslSetting(TSL2591_GAIN_HIGH, TSL2591_INTEGRATIONTIME_600MS, 3277, 62258);
    tslAgt[5] = TslSetting(TSL2591_GAIN_MAX, TSL2591_INTEGRATIONTIME_200MS, 3277, 62258);
    tslAgt[6] = TslSetting(TSL2591_GAIN_MAX, TSL2591_INTEGRATIONTIME_600MS, 3277, 62258);
    setTslAGT(tsl, TSL_SETTINGS_SIZE / 2);
}

void Meteo::setTslAGT(Adafruit_TSL2591 *tsl, int s) {
    tsl->setGain(tslAgt[s].gain);
    tsl->setTiming(tslAgt[s].time);
    tsl->getFullLuminosity();
    vTaskDelay(pdMS_TO_TICKS(AGT_CHANGE_DELAY));
}

TslAutoLum Meteo::getTslAGT(Adafruit_TSL2591 *tsl) {
    int s = TSL_SETTINGS_SIZE / 2;
    uint32_t lum, full;
    while (true) {
        lum = tsl->getFullLuminosity();
        full = lum & 0xFFFF;
        if (full > tslAgt[s].high && s > 0) {
            s--;
            setTslAGT(tsl, s);
            continue;
        }
        if (full < tslAgt[s].low && s < TSL_SETTINGS_SIZE - 1) {
            s++;
            setTslAGT(tsl, s);
            continue;
        }
        break;
    }
    return TslAutoLum(tslAgt[s].gain, tslAgt[s].time, lum);
}

float Meteo::calcSqmAGT(TslAutoLum agt) {
    float lux = calcLuxAGT(agt);

    // Разные варианты конвертации, deprected, удалить позже
    // float sqm = -2.5 * log10(lux) + 18.3;
    // float sqm = (log10(lux) + 5.5917) / -0.40195;
    // float sqm = -2.5 * log10(lux * 0.1466) - 4.0;
    // говорят что это нечто иное, а люксы - ниже
    // float sqm = log10(lux / 108000.) / -0.4;
    // float sqm = log10(lux / 10.8) / -0.4;
    // типа SQM считает вот так? фигня
    // float sqm = -14.0 - 2.5 * log10(lux)
    // Вот такой еще есть вариант
    // float sqm = 21.57 -2.5 * log10(lux);

    // Официальная дока Unihedron
    // https://unihedron.com/projects/sqm-l/Instruction_sheet.pdf
    // https://www.unihedron.com/projects/darksky/cd/SQM-LU-DL/SQM-LU-DL_Users_manual.pdf
    // (в последнем документе есть конверсия NELM<->MPSAS)
    // Получение cd/m2 из mag/arcsec2
    // [cd/m2] = 10.8×104 × 10(-0.4*[mag/arcsec2])
    // Связь канделл на квадратный метр и люксов
    // Для идеально диффузного излучателя (ламбертовского источника), который излучает свет равномерно во всех направлениях полусферы, существует связь:
    // [cd/m2] = [lx]/pi
    // Сведенный результат:
    // [mag/arcsec2] = -2.5 * log10([lx] / 339292)
    // [mag/arcsec2] = -2.5 * log10([lx]) + 13.84 что одно и то же.
    // [lx] = pi * 10.8 * 10^4 * 10^(-0.4 * [mag/arcsec2])
    // [lx] = 339292 * 10^(-0.4 * [mag/arcsec2]) что одно и то же.

    float sqm = -2.5 * log10(lux) + 13.84;

    if (std::isinf(sqm)) {
        sqm = 25.;
    }
    return sqm;
}

float Meteo::calcLuxAGT(TslAutoLum agt) {
    float atime, again;
    float cpl, lux;
    uint16_t ch1 = agt.luminosity >> 16;
    uint16_t ch0 = agt.luminosity & 0xFFFF;
    if ((ch0 == 0xFFFF) | (ch1 == 0xFFFF)) {
        return -1;
    }
    atime = tslTimeAsMillis(agt.time);
    again = tslGainAsMulti(agt.gain);
    // cpl = (ATIME * AGAIN) / DF
    cpl = (atime * again) / TSL2591_LUX_DF;
    // Original lux calculation (for reference sake)
    // float lux1 = ( (float)ch0 - (TSL2591_LUX_COEFB * (float)ch1) ) / cpl;
    // float lux2 = ( ( TSL2591_LUX_COEFC * (float)ch0 ) - ( TSL2591_LUX_COEFD *
    // (float)ch1 ) ) / cpl; lux = lux1 > lux2 ? lux1 : lux2;
    // Alternate lux calculation 1
    // See: https://github.com/adafruit/Adafruit_TSL2591_Library/issues/14
    lux = (((float)ch0 - (float)ch1)) * (1.0F - ((float)ch1 / (float)ch0)) / cpl;
    // Alternate lux calculation 2
    // lux = ( (float)ch0 - ( 1.7F * (float)ch1 ) ) / cpl;
    // Signal I2C had no errors
    return lux;
}