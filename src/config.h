#pragma once

// RTC
#define RTC_TIMEZONE "Europe/Kiev"

// NTP
#define NTP_TIMEZONE TZ_Europe_Kiev
#define NTP_TIMEOUT 5000
#define NTP_INTERVAL_SHORT 10
#define NTP_INTERVAL_LONG 900
#define NTP_SERVER "time.google.com"

// ASCOM Alpaca
#define ALPACA_UDP_PORT 32227
#define ALPACA_TCP_PORT 80

// I2C Sensors
#define I2C_SDA_PIN 5
#define I2C_SCL_PIN 6
#define I2C_MLX_ADDR 0x5A
#define I2C_BMP_ADDR 0x77
#define I2C_AHT_ADDR 0x38

// METEO
#define METEO_MEASURE_DELAY 3000

// LEDS
#define PIN_WIFI_LED 21

// ROOF PINOUT
#define ROOFpin 4