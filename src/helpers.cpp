#include "helpers.h"

String smart_round(float x) {
    float a = fabsf(x);

    if (a >= 10.0f) // целое
        return String(lroundf(x));

    if (a > 1.0f) { // один знак
        String s = String(x, 1);
        if (s.endsWith("0"))
            s.remove(s.length() - 1);
        if (s.endsWith("."))
            s.remove(s.length() - 1);
        return s;
    }

    if (x == 0.0f)
        return "0.00";

    bool neg = x < 0;
    float v = a;
    int sh = 0;

    while (v < 1.0f) {
        v *= 10;
        sh++;
    } // первая значащая
    v = roundf(v * 10) / 10; // две значащие
    while (sh--)
        v /= 10; // вернуть масштаб
    if (neg)
        v = -v;

    String s = String(v, 6); // печатаем
    while (s.endsWith("0"))
        s.remove(s.length() - 1);
    if (s.endsWith("."))
        s.remove(s.length() - 1);
    return s;
}

String uptime() {
    uint64_t uptimeMicros = esp_timer_get_time();
    uint64_t uptimeSeconds = uptimeMicros / 1000000;
    uint32_t days = uptimeSeconds / 86400;
    uint32_t hours = (uptimeSeconds % 86400) / 3600;
    uint32_t minutes = (uptimeSeconds % 3600) / 60;
    uint32_t seconds = uptimeSeconds % 60;
    char uptimeBuffer[50];
    sprintf(uptimeBuffer, "%03lu:%02lu:%02lu:%02lu",
            days, hours, minutes, seconds);
    return String(uptimeBuffer);
}

void faults(int *count, String *description) {
    *description = "";
    *count = 0;
    if (HARDWARE_BMP280 && !INITED_BMP280) {
        if (*count > 0) {
            *description += " ";
        }
        *count++;
        *description += "BMP280";
    }
    if (HARDWARE_AHT20 && !INITED_AHT20) {
        if (*count > 0) {
            *description += " ";
        }
        *count++;
        *description += "AHT20";
    }
    if (HARDWARE_SHT45 && !INITED_SHT45) {
        if (*count > 0) {
            *description += " ";
        }
        *count++;
        *description += "SHT45";
    }
    if (HARDWARE_MLX90614 && !INITED_MLX90614) {
        if (*count > 0) {
            *description += " ";
        }
        *count++;
        *description += "MLX90614";
    }
    if (HARDWARE_TSL2591 && !INITED_TSL2591) {
        if (*count > 0) {
            *description += " ";
        }
        *count++;
        *description += "TSL2591";
    }
    if (HARDWARE_ANEMO4403 && !INITED_ANEMO4403) {
        if (*count > 0) {
            *description += " ";
        }
        *count++;
        *description += "ANEMO4403";
    }
    if (HARDWARE_UICPAL && !INITED_UICPAL) {
        if (*count > 0) {
            *description += " ";
        }
        *count++;
        *description += "UICPAL";
    }
    if (HARDWARE_RG15 && !INITED_RG15) {
        if (*count > 0) {
            *description += " ";
        }
        *count++;
        *description += "RG15";
    }
}
