#include "meteo.h"

Adafruit_BMP280 bmp;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_AHTX0 aht;

Meteo::Meteo(const std::string &newName) : Name(newName) {}

void Meteo::logMessage(String msg, bool showtime) {
    if(logtime && showtime) {
        logger->print(logtime() + " ");
    }
    logger->println(msg);
}

void Meteo::logMessagePart(String msg, bool showtime) {
    if(logtime && showtime) {
        logger->print(logtime() + " ");
    }
    logger->print(msg);
}

void Meteo::setLogger(Stream *stream, std::function<String()> function) {
    logger = stream;
    logtime = function;
}

const std::string &Meteo::getName() const {
    return Name;
}

void Meteo::begin() {
    Wire.end();
    // Set I2C pinout
    Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.begin();
    // Initialize sensors
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
    char buffer[100];
    logMessage(F("[METEO] Update Meteo & Safety Monitors"));

    bmp_temperature = bmp.readTemperature();
    snprintf(buffer, sizeof(buffer), "[METEO][BMP] Temperature: %.1f", bmp_temperature);
    logMessage(buffer);

    bmp_pressure = bmp.readPressure() / 100.0F;
    snprintf(buffer, sizeof(buffer), "[METEO][BMP] Pressure: %.0f", bmp_pressure);
    logMessage(buffer);

    sensors_event_t aht_sensor_humidity, aht_sensor_temp;
    aht.getEvent(&aht_sensor_humidity, &aht_sensor_temp);
    
    aht_temperature = aht_sensor_temp.temperature;
    snprintf(buffer, sizeof(buffer), "[METEO][AHT] Temperature: %.1f", aht_temperature);
    logMessage(buffer);

    aht_humidity = aht_sensor_humidity.relative_humidity;
    snprintf(buffer, sizeof(buffer), "[METEO][AHT] HUMIDITY: %.0f", aht_humidity);
    logMessage(buffer);

    mlx_tempamb = mlx.readAmbientTempC();
    snprintf(buffer, sizeof(buffer), "[METEO][MLX] AMBIENT: %.1f", mlx_tempamb);
    logMessage(buffer);

    mlx_tempobj = mlx.readObjectTempC();
    snprintf(buffer, sizeof(buffer), "[METEO][MLX] OBJECT: %.1f", mlx_tempobj);
    logMessage(buffer);

    dewpoint = aht_temperature - (100 - aht_humidity) / 5.;
    snprintf(buffer, sizeof(buffer), "[METEO][CALC] DEWPOINT: %.1f", dewpoint);
    logMessage(buffer);
    
    tempsky = tsky_calc(mlx_tempobj, mlx_tempamb);
    snprintf(buffer, sizeof(buffer), "[METEO][CALC] SKYTEMP: %.1f", tempsky);
    logMessage(buffer);
    cb_add(tempsky); // add tempsky value to circular buffer and calculate  Turbulence (noise dB) / Seeing estimation
    
    noise_db = cb_noise_db_calc();
    snprintf(buffer, sizeof(buffer), "[METEO][CALC] NOISE: %.1f", noise_db);
    logMessage(buffer);

    cloudcover = 100. + (tempsky * 6.);
    if (cloudcover > 100.)
        cloudcover = 100.;
    if (cloudcover < 0.)
        cloudcover = 0.;
    snprintf(buffer, sizeof(buffer), "[METEO][CALC] CLOUD: %.1f", cloudcover);
    logMessage(buffer);
}