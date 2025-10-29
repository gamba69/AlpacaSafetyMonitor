#pragma once

// General
#define DEVICENAME "AlpacaESP32driver"
#define VERSION "v1.2.1"
#define COPYRIGHT "2021 Njål Brekke & 2023 @agnuca"
#define SAFETYMONITOR2_ENABLE false
#define DEBUG

// ASCOM Alpaca
#define ALPACA_UDP_PORT 32227
#define ALPACA_TCP_PORT 80

// I2C Sensors
#define I2C_SDA_PIN 5
#define I2C_SCL_PIN 6
#define I2C_MLX_ADDR 0x5A
#define I2C_BMP_ADDR 0x77
#define I2C_AHT_ADDR 0x38

// LEDS
#define PIN_WIFI_LED 21

// ROOF PINOUT
#define ROOFpin 4