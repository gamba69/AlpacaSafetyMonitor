#include "helpers.h"

String smart_round(float x) {
    float a = fabsf(x);

    if (a >= 10.0f)               // целое
        return String(lroundf(x));

    if (a > 1.0f) {               // один знак
        String s = String(x, 1);
        if (s.endsWith("0")) s.remove(s.length() - 1);
        if (s.endsWith(".")) s.remove(s.length() - 1);
        return s;
    }

    if (x == 0.0f) return "0.00";

    bool neg = x < 0;
    float v = a;
    int sh = 0;

    while (v < 1.0f) { v *= 10; sh++; }   // первая значащая
    v = roundf(v * 10) / 10;              // две значащие
    while (sh--) v /= 10;                 // вернуть масштаб
    if (neg) v = -v;

    String s = String(v, 6);              // печатаем
    while (s.endsWith("0")) s.remove(s.length() - 1);
    if (s.endsWith(".")) s.remove(s.length() - 1);
    return s;
}