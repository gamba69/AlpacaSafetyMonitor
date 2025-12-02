#include "meteo.h"
#include "hardware.h"

#define sgn(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))

float Meteo::tsky_calc(float ts, float ta) {
    float t67, td = 0;
    float k[] = {0., 33., 0., 4., 100., 100., 0., 0.};
    if (abs(k[2] / 10. - ta) < 1) {
        t67 = sgn(k[6]) * sgn(ta - k[2] / 10.) * abs((k[2] / 10. - ta));
    } else {
        t67 = k[6] * sgn(ta - k[2] / 10.) * (log(abs((k[2] / 10 - ta))) / log(10.) + k[7] / 100);
    }
    td = (k[1] / 100.) * (ta - k[2] / 10.) + (k[3] / 100.) * pow(exp(k[4] / 1000. * ta), (k[5] / 100.)) + t67;
    return (ts - td);
}

float Meteo::cb_avg_calc() {
    int sum = 0;
    for (int i = 0; i < CB_SIZE; i++)
        sum += cb[i];
    return ((float)sum) / CB_SIZE;
}

float Meteo::cb_rms_calc() {
    int sum = 0;
    for (int i = 0; i < CB_SIZE; i++)
        sum += cb[i] * cb[i];
    return sqrt(sum / CB_SIZE);
}

void Meteo::cb_add(float value) {
    cb[cb_index] = value;
    cb_avg = cb_avg_calc();
    cb_rms = cb_rms_calc();
    cb_noise[cb_index] = abs(value) - cb_rms;
    cb_index++;
    if (cb_index == CB_SIZE)
        cb_index = 0;
}

float Meteo::cb_noise_db_calc() {
    float n = 0;
    for (int i = 0; i < CB_SIZE; i++) {
        n += cb_noise[i] * cb_noise[i];
    }
    if (n == 0)
        return 0;
    return (10 * log10(n));
}

float Meteo::cb_snr_calc() {
    float s, n = 0;
    for (int i = 0; i < CB_SIZE; i++) {
        s += cb[i] * cb[i];
        n += cb_noise[i] * cb_noise[i];
    }
    if (n == 0)
        return 0;
    return (10 * log10(s / n));
}
