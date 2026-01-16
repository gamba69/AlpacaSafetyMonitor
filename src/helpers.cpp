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
    }                        // первая значащая
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