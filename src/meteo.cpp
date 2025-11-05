#include "meteo.h"

Adafruit_BMP280 bmp;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_AHTX0 aht;

Meteo::Meteo(const std::string &newName) : Name(newName) {}

void Meteo::logMessage(String msg, bool showtime) {
    if (logLine && logLinePart) {
        if (logTime && showtime) {
            logLinePart(logTime() + " ");
        }
        logLine(msg);
    }
}

void Meteo::logMessagePart(String msg, bool showtime) {
    if (logLinePart) {
        if (logTime && showtime) {
            logLinePart(logTime() + " ");
        }
        logLinePart(msg);
    }
}

void Meteo::setLogger(std::function<void(String)> logLineCallback, std::function<void(String)> logLinePartCallback, std::function<String()> logTimeCallback) {
    logLine = logLineCallback;
    logLinePart = logLinePartCallback;
    logTime = logTimeCallback;
}

const std::string &Meteo::getName() const {
    return Name;
}

void Meteo::begin() {
    pinMode(RAIN_SENSOR_PIN, INPUT_PULLDOWN);
    Wire.end();
    Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.begin();
    mlx.begin(I2C_MLX_ADDR);
    bmp.begin(I2C_BMP_ADDR);
    aht.begin(&Wire, 0, I2C_AHT_ADDR);
}

#define sgn(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))

float tsky_calc(float ts, float ta) {
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

float cb_avg_calc() {
    int sum = 0;
    for (int i = 0; i < CB_SIZE; i++)
        sum += cb[i];
    return ((float)sum) / CB_SIZE;
}

float cb_rms_calc() {
    int sum = 0;
    for (int i = 0; i < CB_SIZE; i++)
        sum += cb[i] * cb[i];
    return sqrt(sum / CB_SIZE);
}

void cb_add(float value) {
    cb[cb_index] = value;
    cb_avg = cb_avg_calc();
    cb_rms = cb_rms_calc();
    cb_noise[cb_index] = abs(value) - cb_rms;
    cb_index++;
    if (cb_index == CB_SIZE)
        cb_index = 0;
}

float cb_noise_db_calc() {
    float n = 0;
    for (int i = 0; i < CB_SIZE; i++) {
        n += cb_noise[i] * cb_noise[i];
    }
    if (n == 0)
        return 0;
    return (10 * log10(n));
}

float cb_snr_calc() {
    float s, n = 0;
    for (int i = 0; i < CB_SIZE; i++) {
        s += cb[i] * cb[i];
        n += cb_noise[i] * cb_noise[i];
    }
    if (n == 0)
        return 0;
    return (10 * log10(s / n));
}

// Read through i2c bus

void Meteo::update(unsigned long measureDelay) {
    logMessage(F("[METEO] Updating Meteo monitors"));
    bmp_temperature = bmp.readTemperature();
    bmp_pressure = bmp.readPressure() / 100.0F;
    sensors_event_t aht_sensor_humidity, aht_sensor_temp;
    aht.getEvent(&aht_sensor_humidity, &aht_sensor_temp);
    aht_temperature = aht_sensor_temp.temperature;
    aht_humidity = aht_sensor_humidity.relative_humidity;
    mlx_tempamb = mlx.readAmbientTempC();
    mlx_tempobj = mlx.readObjectTempC();
    dewpoint = aht_temperature - (100 - aht_humidity) / 5.;
    tempsky = tsky_calc(mlx_tempobj, mlx_tempamb);
    cb_add(tempsky); // add tempsky value to circular buffer and calculate  Turbulence (noise dB) / Seeing estimation
    noise_db = cb_noise_db_calc();
    cloudcover = 100. + (tempsky * 6.);
    if (cloudcover > 100.)
        cloudcover = 100.;
    if (cloudcover < 0.)
        cloudcover = 0.;
    if (digitalRead(RAIN_SENSOR_PIN)) {
        rainrate = 1;
    } else {
        rainrate = 0;
    }
    logMessage("[METEO][BMP] Temperature: " + String(bmp_temperature, 1) + " °C");
    logMessage("[METEO][BMP] Pressure: " + String(bmp_pressure, 0) + " hPa");
    logMessage("[METEO][AHT] Temperature: " + String(aht_temperature, 1) + " °C");
    logMessage("[METEO][AHT] Humidity: " + String(aht_humidity, 0) + " %");
    logMessage("[METEO][MLX] Ambient: " + String(mlx_tempamb, 1) + " °C");
    logMessage("[METEO][MLX] Object: " + String(mlx_tempobj, 1) + " °C");
    logMessage("[METEO][CALC] Dewpoint: " + String(dewpoint, 1) + " °C");
    logMessage("[METEO][CALC] Sky temperature: " + String(tempsky, 1) + " °C");
    logMessage("[METEO][CALC] Cloud cover: " + String(cloudcover, 0) + " %");
    logMessage("[METEO][CALC] Turbulence: " + String(noise_db, 1) + " dB");
    logMessage("[METEO][RAIN] Rain rate: " + String(rainrate, 1) + " mm/h");
}