#include "console.h"
#include "calibrate.h"
#include "hardware.h"
#include "helpers.h"
#include "log.h"
#include "weights.h"
#include <Arduino.h>
#include <algorithm>
#include <iterator>
#include <map>
#include <sstream>
#include <string>

std::map<std::string, std::function<void()>> console_commands;

void commandHelp() {
    logConsoleMessage("[HELP] Use the following commands");
    logConsoleMessage("[HELP] --------------------------");
    logConsoleMessage("[HELP]   help         - this screen");
    logConsoleMessage("[HELP]   help info    - show help about informational commands");
    logConsoleMessage("[HELP]   help general - show help about general commands");
    logConsoleMessage("[HELP]   help log     - show help about log settings");
    logConsoleMessage("[HELP]   help target  - show help about log target settings");
    logConsoleMessage("[HELP]   help hw      - show help about hardware settings");
    logConsoleMessage("[HELP]   help alpaca  - show help about alpaca settings");
    logConsoleMessage("[HELP]   help temp    - show help about temperature calc weights");
    logConsoleMessage("[HELP]   help humi    - show help about humidity calc weights");
}

void commandHelpInfo() {
    logConsoleMessage("[HELP] Available console commands");
    logConsoleMessage("[HELP] --------------------------");
    logConsoleMessage("[HELP] Info:");
    logConsoleMessage("[HELP]   log    - show curent log settings");
    logConsoleMessage("[HELP]   target - show curent log target settings");
    logConsoleMessage("[HELP]   hw     - show curent hw settings");
    logConsoleMessage("[HELP]   temp   - show curent temperature calc weights");
    logConsoleMessage("[HELP]   humi   - show curent humidity calc weights");
    logConsoleMessage("[HELP]   uptime - show curent system uptime");
    logConsoleMessage("[HELP]   faults - show curent sensor faults");
    logConsoleMessage("[HELP] General:");
    logConsoleMessage("[HELP]   reboot - restart esp32 ascom alpaca device");
}

void commandHelpGeneral() {
    commandHelpInfo();
}

void commandHelpLog() {
    logConsoleMessage("[HELP] Available console commands");
    logConsoleMessage("[HELP] --------------------------");
    logConsoleMessage("[HELP] Log settings:");
    logConsoleMessage("[HELP]   log                   - show curent log settings");
    logConsoleMessage("[HELP]   log on/slow/off               - enable/disable all logging");
    logConsoleMessage("[HELP]   log main on/off               - enable/disable main logging");
    logConsoleMessage("[HELP]   log alpaca on/off             - enable/disable alpaca server logging");
    logConsoleMessage("[HELP]   log meteo on/slow [nnn]/off   - enable/slow, nnn seconds/disable meteo logging");
    logConsoleMessage("[HELP]   log obscon on/slow [nnn]/off  - enable/slow, nnn seconds/disable observing conditions logging");
    logConsoleMessage("[HELP]   log safemon on/slow [nnn]/off - enable/slow, nnn seconds/disable safety monitor logging");
    logConsoleMessage("[HELP]   log wifi on/off               - enable/disable wifi manager logging");
    logConsoleMessage("[HELP]   log ota on/off                - enable/disable ota update logging");
    logConsoleMessage("[HELP]   log tech on/off               - enable/disable tech sensor data logging");
    logConsoleMessage("[HELP] Log target settings:");
    logConsoleMessage("[HELP]   target                - show curent log target settings");
    logConsoleMessage("[HELP]   target serial on/off  - enable/disable serial log output");
    logConsoleMessage("[HELP]   target console on/off - enable/disable console log output");
    logConsoleMessage("[HELP]   target mqtt on/off    - enable/disable mqtt log output");
    logConsoleMessage("[HELP]   target led on/off     - enable/disable led log output");
}

void commandHelpTarget() {
    commandHelpLog();
}

void commandHelpHw() {
    logConsoleMessage("[HELP] Available console commands");
    logConsoleMessage("[HELP] --------------------------");
    logConsoleMessage("[HELP] Hardware settings (reboot required):");
    logConsoleMessage("[HELP]   hw                  - show curent hw settings");
    logConsoleMessage("[HELP]   hw bmp280 on/off    - enable/disable BMP280 sensor");
    logConsoleMessage("[HELP]   hw aht20 on/off     - enable/disable AHT20 sensor");
    logConsoleMessage("[HELP]   hw sht45 on/off     - enable/disable SHT45 sensor");
    logConsoleMessage("[HELP]   hw mlx90614 on/off  - enable/disable MLX90614 sensor");
    logConsoleMessage("[HELP]   hw tsl2591 on/off   - enable/disable TSL2591 sensor");
    logConsoleMessage("[HELP]   hw anemo4403 on/off - enable/disable ANEMO4403 sensor");
    logConsoleMessage("[HELP]   hw uicpal on/off    - enable/disable UICPAL sensor");
    logConsoleMessage("[HELP]   hw rg15 on/off      - enable/disable RG-15 sensor");
    logConsoleMessage("[HELP] Alpaca settings (reboot required):");
    logConsoleMessage("[HELP]   alpaca obscon on/off  - enable/disable observing conditions service");
    logConsoleMessage("[HELP]   alpaca safemon on/off - enable/disable safety monitor service");
}

void commandHelpAlpaca() {
    commandHelpHw();
}

void commandHelpTemp() {
    logConsoleMessage("[HELP] Available console commands");
    logConsoleMessage("[HELP] --------------------------");
    logConsoleMessage("[HELP] Temperature calc weights:");
    logConsoleMessage("[HELP]   temp          - show curent temperature calc weights");
    logConsoleMessage("[HELP]   temp sht n.nn - set SHT45 temperature calc weight");
    logConsoleMessage("[HELP]   temp aht n.nn - set AHT20 temperature calc weight");
    logConsoleMessage("[HELP]   temp bmp n.nn - set BMP280 temperature calc weight");
    logConsoleMessage("[HELP]   temp weight n.nn n.nn n.nn - set SHT45/AHT20/BMP280 temperature calc weights at once");
    logConsoleMessage("[HELP] Humidity calc weights:");
    logConsoleMessage("[HELP]   humi          - show curent humidity calc weights");
    logConsoleMessage("[HELP]   humi sht n.nn - set SHT45 humidity calc weight");
    logConsoleMessage("[HELP]   humi aht n.nn - set AHT20 humidity calc weight");
    logConsoleMessage("[HELP]   humi weight n.nn n.nn - set SHT45/AHT20 humidity calc weights at once");
}

void commandHelpHumi() {
    commandHelpHumi();
}

void commandLogState() {
    logConsoleMessage("[INFO] Logging state");
    logConsoleMessage("[INFO] -------------");
    logConsoleMessage("[INFO]  console - on");
    logConsoleMessage("[INFO]  main    - " + String(logEnabled[LogSource::Main] ? "on" : "off"));
    logConsoleMessage("[INFO]  alpaca  - " + String(logEnabled[LogSource::Alpaca] ? "on" : "off"));
    logConsoleMessage("[INFO]  meteo   - " + String(logEnabled[LogSource::Meteo] ? (logEnabled[LogSource::Meteo] == Log::On ? "on" : ("slow " + String(logSlow[LogSource::Meteo]))) : "off") + " [meteo sensors]");
    logConsoleMessage("[INFO]  obscon  - " + String(logEnabled[LogSource::ObsCon] ? (logEnabled[LogSource::ObsCon] == Log::On ? "ob" : ("slow " + String(logSlow[LogSource::ObsCon]))) : "off") + " [observing conditions]");
    logConsoleMessage("[INFO]  safemon - " + String(logEnabled[LogSource::SafeMon] ? (logEnabled[LogSource::SafeMon] == Log::On ? "on" : ("slow " + String(logSlow[LogSource::SafeMon]))) : "off") + " [safety monitor]");
    logConsoleMessage("[INFO]  wifi    - " + String(logEnabled[LogSource::Wifi] ? "on" : "off") + " [wifi manager]");
    logConsoleMessage("[INFO]  ota     - " + String(logEnabled[LogSource::Ota] ? "on" : "off") + " [update manager]");
    logConsoleMessage("[INFO]  tech    - " + String(logEnabled[LogSource::Tech] ? "on" : "off") + " [tech sensor data]");
}

void commandTargetState() {
    logConsoleMessage("[INFO] Logging target state");
    logConsoleMessage("[INFO] --------------------");
    logConsoleMessage("[INFO]  serial  - " + String(LOG_SERIAL ? "on" : "off"));
    logConsoleMessage("[INFO]  console - " + String(LOG_CONSOLE ? "on" : "off"));
    logConsoleMessage("[INFO]  mqtt    - " + String(LOG_MQTT ? "on" : "off"));
    logConsoleMessage("[INFO]  led     - " + String(LOG_LED ? "on" : "off"));
}

void commandHardwareState() {
    logConsoleMessage("[INFO] Firmware supported hardware");
    logConsoleMessage("[INFO] ---------------------------");
    logConsoleMessage("[INFO] Sensors:");
    logConsoleMessage("[INFO]   DS3231    - " + String(HARDWARE_DS3231 ? "enabled " : "disabled") + " (realtime clock)");
    logConsoleMessage("[INFO]   BMP280    - " + String(HARDWARE_BMP280 ? "enabled " : "disabled") + String(HARDWARE_BMP280 && INITED_BMP280 ? ", OK" : ", FAULT") + " (temperature and pressure)");
    logConsoleMessage("[INFO]   AHT20     - " + String(HARDWARE_AHT20 ? "enabled " : "disabled") + String(HARDWARE_AHT20 && INITED_AHT20 ? ", OK" : ", FAULT") + " (temperature and humidity)");
    logConsoleMessage("[INFO]   SHT45     - " + String(HARDWARE_SHT45 ? "enabled " : "disabled") + String(HARDWARE_SHT45 && INITED_SHT45 ? ", OK" : ", FAULT") + " (temperature and humidity)");
    logConsoleMessage("[INFO]   MLX90614  - " + String(HARDWARE_MLX90614 ? "enabled " : "disabled") + String(HARDWARE_MLX90614 && INITED_MLX90614 ? ", OK" : ", FAULT") + " (sky temperature)");
    logConsoleMessage("[INFO]   TSL2591   - " + String(HARDWARE_TSL2591 ? "enabled " : "disabled") + String(HARDWARE_TSL2591 && INITED_TSL2591 ? ", OK" : ", FAULT") + " (sky brightness)");
    logConsoleMessage("[INFO]   ANEMO4403 - " + String(HARDWARE_ANEMO4403 ? "enabled " : "disabled") + String(HARDWARE_ANEMO4403 && INITED_ANEMO4403 ? ", OK" : ", FAULT") + " (wind speed)");
    logConsoleMessage("[INFO]   UICPAL    - " + String(HARDWARE_UICPAL ? "enabled " : "disabled") + String(HARDWARE_UICPAL && INITED_UICPAL ? ", OK" : ", FAULT") + " (rain/snow sensor)");
    logConsoleMessage("[INFO]   RG15      - " + String(HARDWARE_RG15 ? "enabled " : "disabled") + String(HARDWARE_RG15 && INITED_RG15 ? ", OK" : ", FAULT") + " (rain rate sensor)");
    logConsoleMessage("[INFO] Alpaca:");
    logConsoleMessage("[INFO]   Observing conditions - " + String(ALPACA_OBSCON ? "enabled" : "disabled"));
    logConsoleMessage("[INFO]   Safety monitor       - " + String(ALPACA_SAFEMON ? "enabled" : "disabled"));
    logConsoleMessage("[INFO] Obsering conditions:");
    logConsoleMessage("[INFO]   Rain rate       - " + String(OBSCON_RAINRATE ? "enabled" : "disabled"));
    logConsoleMessage("[INFO]   Temperature     - " + String(OBSCON_TEMPERATURE ? "enabled" : "disabled"));
    logConsoleMessage("[INFO]   Humidity        - " + String(OBSCON_HUMIDITY ? "enabled" : "disabled"));
    logConsoleMessage("[INFO]   Dew point       - " + String(OBSCON_DEWPOINT ? "enabled" : "disabled"));
    logConsoleMessage("[INFO]   Pressure        - " + String(OBSCON_PRESSURE ? "enabled" : "disabled"));
    logConsoleMessage("[INFO]   Sky temperature - " + String(OBSCON_SKYTEMP ? "enabled" : "disabled"));
    logConsoleMessage("[INFO]   Cloud cover     - " + String(OBSCON_CLOUDCOVER ? "enabled" : "disabled"));
    logConsoleMessage("[INFO]   Star FWHM       - " + String(OBSCON_FWHM ? "enabled" : "disabled"));
    logConsoleMessage("[INFO]   Sky brightness  - " + String(OBSCON_SKYBRIGHTNESS ? "enabled" : "disabled"));
    logConsoleMessage("[INFO]   Sky quality     - " + String(OBSCON_SKYQUALITY ? "enabled" : "disabled"));
    logConsoleMessage("[INFO]   Wind direction  - " + String(OBSCON_WINDDIR ? "enabled" : "disabled"));
    logConsoleMessage("[INFO]   Wind speed      - " + String(OBSCON_WINDSPEED ? "enabled" : "disabled"));
    logConsoleMessage("[INFO]   Wind gust       - " + String(OBSCON_WINDGUST ? "enabled" : "disabled"));
    logConsoleMessage("[INFO] Safety monitor:");
    logConsoleMessage("[INFO]   Rain rate       - " + String(SAFEMON_RAINRATE ? "enabled" : "disabled"));
    logConsoleMessage("[INFO]   Temperature     - " + String(SAFEMON_TEMPERATURE ? "enabled" : "disabled"));
    logConsoleMessage("[INFO]   Humidity        - " + String(SAFEMON_HUMIDITY ? "enabled" : "disabled"));
    logConsoleMessage("[INFO]   Dew point       - " + String(SAFEMON_DEWPOINT ? "enabled" : "disabled"));
    logConsoleMessage("[INFO]   Sky temperature - " + String(SAFEMON_SKYTEMP ? "enabled" : "disabled"));
    logConsoleMessage("[INFO]   Wind speed      - " + String(SAFEMON_WINDSPEED ? "enabled" : "disabled"));
}

String calCoeffAsString(CalCoefficient c) {
    String result = "y = ";
    if (c.a != 1) {
        result += String(c.a) + " * ";
    }
    result += "x ";
    if (c.b != 0) {
        if (c.b > 0) {
            result += "+ " + String(c.b);
        } else {
            result += "- " + String(abs(c.b));
        }
    }
    return result;
}

void commandCalibrateState() {
    logConsoleMessage("[INFO] Calibration coefficients");
    logConsoleMessage("[INFO] ------------------------");
    logConsoleMessage("[INFO]   BMP280 Temperature       - " + calCoeffAsString(CAL_BMP280_TEMPERATURE));
    logConsoleMessage("[INFO]   BMP280 Pressure          - " + calCoeffAsString(CAL_BMP280_PRESSURE));
    logConsoleMessage("[INFO]   AHT20 Temperature        - " + calCoeffAsString(CAL_AHT20_TEMPERATURE));
    logConsoleMessage("[INFO]   AHT20 Humidity           - " + calCoeffAsString(CAL_AHT20_HUMIDITY));
    logConsoleMessage("[INFO]   SHT45 Temperature        - " + calCoeffAsString(CAL_SHT45_TEMPERATURE));
    logConsoleMessage("[INFO]   SHT45 Humidity           - " + calCoeffAsString(CAL_SHT45_HUMIDITY));
    logConsoleMessage("[INFO]   Dew Point                - " + calCoeffAsString(CAL_DEW_POINT));
    logConsoleMessage("[INFO]   MLX90614 Ambient         - " + calCoeffAsString(CAL_MLX90614_AMBIENT));
    logConsoleMessage("[INFO]   MLX90614 Object          - " + calCoeffAsString(CAL_MLX90614_OBJECT));
    logConsoleMessage("[INFO]   MLX90614 Sky Temperature - " + calCoeffAsString(CAL_MLX90614_SKYTEMP));
    logConsoleMessage("[INFO]   MLX90614 Cloud Cover     - " + calCoeffAsString(CAL_MLX90614_CLOUDCOVER));
    logConsoleMessage("[INFO]   TSL2591 Sky Brightness   - " + calCoeffAsString(CAL_TSL2591_SKYBRIGHTNESS));
    logConsoleMessage("[INFO]   TSL2591 Sky Quality      - " + calCoeffAsString(CAL_TSL2591_SKYQUALITY));
    logConsoleMessage("[INFO]   ANEMO4403 Wind Speed     - " + calCoeffAsString(CAL_ANEMO4403_WINDSPEED));
    logConsoleMessage("[INFO]   ANEMO4403 Wind Gust      - " + calCoeffAsString(CAL_ANEMO4403_WINDGUST));
    logConsoleMessage("[INFO]   UICPAL Rain Rate         - " + calCoeffAsString(CAL_UICPAL_RAINRATE));
    logConsoleMessage("[INFO]   RG15 Rain Rate           - " + calCoeffAsString(CAL_RG15_RAINRATE));
}

void commandTempWeightState() {
    logConsoleMessage("[INFO] Temperature calc weights");
    logConsoleMessage("[INFO] ------------------------");
    logConsoleMessage("[INFO]  SHT45  - " + String(T_NORM_WEIGHT_SHT45) + " [" + String(T_WEIGHT_SHT45) + "]");
    logConsoleMessage("[INFO]  AHT20  - " + String(T_NORM_WEIGHT_AHT20) + " [" + String(T_WEIGHT_AHT20) + "]");
    logConsoleMessage("[INFO]  BMP280 - " + String(T_NORM_WEIGHT_BMP280) + " [" + String(T_WEIGHT_BMP280) + "]");
}

void commandHumiWeightState() {
    logConsoleMessage("[INFO] Humidity calc weights");
    logConsoleMessage("[INFO] ---------------------");
    logConsoleMessage("[INFO]  SHT45  - " + String(H_NORM_WEIGHT_SHT45) + " [" + String(H_WEIGHT_SHT45) + "]");
    logConsoleMessage("[INFO]  AHT20  - " + String(H_NORM_WEIGHT_AHT20) + " [" + String(H_WEIGHT_AHT20) + "]");
}

void commandUptime() {
    logConsoleMessage("[INFO] Uptime");
    logConsoleMessage("[INFO] ------------");
    logConsoleMessage("[INFO] " + uptime());
}

void commandFaults() {
    int count;
    String descr;
    faults(&count, &descr);
    logConsoleMessage("[INFO] Faults");
    logConsoleMessage("[INFO] ------------");
    if (count > 0) {
        logConsoleMessage("[INFO] " + String(count) + ": " + descr);
    } else {
        logConsoleMessage("[INFO] None");
    }
}

void commandReboot() {
    logConsoleMessage("[CONSOLE] Immediate reboot requested!");
    logConsoleMessage("[REBOOT]");
    delay(1000);
    ESP.restart();
}

void commandCalibrateBMP280Temperature(float a, float b) {
    CalCoefficient c(a, b);
    CAL_BMP280_TEMPERATURE = c;
    saveCalPrefs();
}

void commandCalibrateBMP280Pressure(float a, float b) {
    CalCoefficient c(a, b);
    CAL_BMP280_PRESSURE = c;
    saveCalPrefs();
}

void commandCalibrateAHT20Temperature(float a, float b) {
    CalCoefficient c(a, b);
    CAL_AHT20_TEMPERATURE = c;
    saveCalPrefs();
}

void commandCalibrateAHT20Humidity(float a, float b) {
    CalCoefficient c(a, b);
    CAL_AHT20_HUMIDITY = c;
    saveCalPrefs();
}

void commandCalibrateSHT45Temperature(float a, float b) {
    CalCoefficient c(a, b);
    CAL_SHT45_TEMPERATURE = c;
    saveCalPrefs();
}

void commandCalibrateSHT45Humidity(float a, float b) {
    CalCoefficient c(a, b);
    CAL_SHT45_TEMPERATURE = c;
    saveCalPrefs();
}

void commandCalibrateDewPoint(float a, float b) {
    CalCoefficient c(a, b);
    CAL_DEW_POINT = c;
    saveCalPrefs();
}

void commandCalibrateMLX90614Ambient(float a, float b) {
    CalCoefficient c(a, b);
    CAL_MLX90614_AMBIENT = c;
    saveCalPrefs();
}

void commandCalibrateMLX90614Object(float a, float b) {
    CalCoefficient c(a, b);
    CAL_MLX90614_OBJECT = c;
    saveCalPrefs();
}

void commandCalibrateMLX90614SkyTemperature(float a, float b) {
    CalCoefficient c(a, b);
    CAL_MLX90614_SKYTEMP = c;
    saveCalPrefs();
}

void commandCalibrateMLX90614CloudCover(float a, float b) {
    CalCoefficient c(a, b);
    CAL_MLX90614_CLOUDCOVER = c;
    saveCalPrefs();
}

void commandCalibrateTSL2591SkyBrightness(float a, float b) {
    CalCoefficient c(a, b);
    CAL_TSL2591_SKYBRIGHTNESS = c;
    saveCalPrefs();
}

void commandCalibrateTSL2591SkyQuality(float a, float b) {
    CalCoefficient c(a, b);
    CAL_TSL2591_SKYQUALITY = c;
    saveCalPrefs();
}

void commandCalibrateANEMO4403WindSpeed(float a, float b) {
    CalCoefficient c(a, b);
    CAL_ANEMO4403_WINDSPEED = c;
    saveCalPrefs();
}

void commandCalibrateANEMO4403WindGust(float a, float b) {
    CalCoefficient c(a, b);
    CAL_ANEMO4403_WINDGUST = c;
    saveCalPrefs();
}

void commandCalibrateUICPALRainRate(float a, float b) {
    CalCoefficient c(a, b);
    CAL_UICPAL_RAINRATE = c;
    saveCalPrefs();
}

void commandCalibrateRG15RainRate(float a, float b) {
    CalCoefficient c(a, b);
    CAL_RG15_RAINRATE = c;
    saveCalPrefs();
}

void commandTargetSerialOn() {
    logConsoleMessage("[CONSOLE] Serial target logging enabled");
    LOG_SERIAL = Log::On;
    saveLogPrefs();
}

void commandTargetSerialOff() {
    logConsoleMessage("[CONSOLE] Serial target logging disabled");
    LOG_SERIAL = Log::Off;
    saveLogPrefs();
}

void commandTargetConsoleOn() {
    logConsoleMessage("[CONSOLE] Console target logging enabled");
    LOG_CONSOLE = Log::On;
    saveLogPrefs();
}

void commandTargetConsoleOff() {
    logConsoleMessage("[CONSOLE] Console target logging disabled");
    LOG_CONSOLE = Log::Off;
    saveLogPrefs();
}

void commandTargetMqttOn() {
    logConsoleMessage("[CONSOLE] MQTT target logging enabled");
    LOG_MQTT = Log::On;
    saveLogPrefs();
}

void commandTargetMqttOff() {
    logConsoleMessage("[CONSOLE] MQTT target logging disabled");
    LOG_MQTT = Log::Off;
    saveLogPrefs();
}

void commandTargetLedOn() {
    logConsoleMessage("[CONSOLE] Led target logging enabled");
    LOG_LED = Log::On;
    saveLogPrefs();
}

void commandTargetLedOff() {
    logConsoleMessage("[CONSOLE] Led target logging disabled");
    LOG_LED = Log::Off;
    saveLogPrefs();
}

void commandLogMainOn() {
    logConsoleMessage("[CONSOLE] Main logging enabled");
    logEnabled[LogSource::Main] = Log::On;
    saveLogPrefs();
}

void commandLogMainOff() {
    logConsoleMessage("[CONSOLE] Main logging disabled");
    logEnabled[LogSource::Main] = Log::Off;
    saveLogPrefs();
}

void commandLogMeteoOn() {
    logConsoleMessage("[CONSOLE] Meteo logging enabled");
    logEnabled[LogSource::Meteo] = Log::On;
    saveLogPrefs();
}

void commandLogMeteoOff() {
    logConsoleMessage("[CONSOLE] Meteo logging disabled");
    logEnabled[LogSource::Meteo] = Log::Off;
    saveLogPrefs();
}

void commandLogMeteoSlow() {
    logConsoleMessage("[CONSOLE] Meteo logging slow " + logSlow[LogSource::Meteo]);
    logEnabled[LogSource::Meteo] = Log::Slow;
    saveLogPrefs();
}

void commandLogMeteoSlowDelay(uint16_t delay) {
    logConsoleMessage("[CONSOLE] Meteo logging slow " + String(delay));
    logEnabled[LogSource::Meteo] = Log::Slow;
    logSlow[LogSource::Meteo] = delay;
    saveLogPrefs();
}

void commandLogAlpacaOn() {
    logConsoleMessage("[CONSOLE] Alpaca logging enabled");
    logEnabled[LogSource::Alpaca] = Log::On;
    saveLogPrefs();
}

void commandLogAlpacaOff() {
    logConsoleMessage("[CONSOLE] Alpaca logging disabled");
    logEnabled[LogSource::Alpaca] = Log::Off;
    saveLogPrefs();
}

void commandLogObsconOn() {
    logConsoleMessage("[CONSOLE] Observing conditions logging enabled");
    logEnabled[LogSource::ObsCon] = Log::On;
    saveLogPrefs();
}

void commandLogObsconOff() {
    logConsoleMessage("[CONSOLE] Observing conditions logging disabled");
    logEnabled[LogSource::ObsCon] = Log::Off;
    saveLogPrefs();
}

void commandLogObsconSlow() {
    logConsoleMessage("[CONSOLE] Observing conditions logging slow " + String(logSlow[LogSource::ObsCon]));
    logEnabled[LogSource::ObsCon] = Log::Slow;
    saveLogPrefs();
}

void commandLogObsconSlowDelay(uint16_t delay) {
    logConsoleMessage("[CONSOLE] Observing conditions logging slow " + String(delay));
    logEnabled[LogSource::ObsCon] = Log::Slow;
    logSlow[LogSource::ObsCon] = delay;
    saveLogPrefs();
}

void commandLogSafemonOn() {
    logConsoleMessage("[CONSOLE] Safety monitor logging enabled");
    logEnabled[LogSource::SafeMon] = Log::On;
    saveLogPrefs();
}

void commandLogSafemonOff() {
    logConsoleMessage("[CONSOLE] Safety monitor logging disabled");
    logEnabled[LogSource::SafeMon] = Log::Off;
    saveLogPrefs();
}

void commandLogSafemonSlow() {
    logConsoleMessage("[CONSOLE] Safety monitor logging slow " + String(logSlow[LogSource::SafeMon]));
    logEnabled[LogSource::SafeMon] = Log::Slow;
    saveLogPrefs();
}

void commandLogSafemonSlowDelay(uint16_t delay) {
    logConsoleMessage("[CONSOLE] Safety monitor logging slow " + String(delay));
    logEnabled[LogSource::SafeMon] = Log::Slow;
    logSlow[LogSource::SafeMon] = delay;
    saveLogPrefs();
}

void commandLogWifiOn() {
    logConsoleMessage("[CONSOLE] Wifi monitor logging enabled");
    logEnabled[LogSource::Wifi] = Log::On;
    saveLogPrefs();
}

void commandLogWifiOff() {
    logConsoleMessage("[CONSOLE] Wifi logging disabled");
    logEnabled[LogSource::Wifi] = Log::Off;
    saveLogPrefs();
}

void commandLogOtaOn() {
    logConsoleMessage("[CONSOLE] Ota monitor logging enabled");
    logEnabled[LogSource::Ota] = Log::On;
    saveLogPrefs();
}

void commandLogOtaOff() {
    logConsoleMessage("[CONSOLE] Ota logging disabled");
    logEnabled[LogSource::Ota] = Log::Off;
    saveLogPrefs();
}

void commandLogTechOn() {
    logConsoleMessage("[CONSOLE] Tech sensor data logging enabled");
    logEnabled[LogSource::Tech] = Log::On;
    saveLogPrefs();
}

void commandLogTechOff() {
    logConsoleMessage("[CONSOLE] Tech sensor data logging disabled");
    logEnabled[LogSource::Tech] = Log::Off;
    saveLogPrefs();
}

void commandLogOn() {
    logConsoleMessage("[CONSOLE] All logging enabled");
    std::fill(std::begin(logEnabled), std::end(logEnabled), Log::On);
    saveLogPrefs();
}

void commandLogOff() {
    logConsoleMessage("[CONSOLE] All logging disabled");
    std::fill(std::begin(logEnabled), std::end(logEnabled), Log::Off);
    saveLogPrefs();
}

void commandLogSlow() {
    logConsoleMessage("[CONSOLE] All logging enabled, slow");
    std::fill(std::begin(logEnabled), std::end(logEnabled), Log::On);
    logEnabled[LogSource::Meteo] = Log::Slow;
    logEnabled[LogSource::ObsCon] = Log::Slow;
    logEnabled[LogSource::SafeMon] = Log::Slow;
    saveLogPrefs();
}

void commandAlpacaObsconOn() {
    logConsoleMessage("[CONSOLE] Alpaca observing conditions enabled");
    ALPACA_OBSCON = true;
    saveHwPrefs();
}

void commandAlpacaObsconOff() {
    logConsoleMessage("[CONSOLE] Alpaca observing conditions disabled");
    ALPACA_OBSCON = false;
    saveHwPrefs();
}

void commandAlpacaSafemonOn() {
    logConsoleMessage("[CONSOLE] Alpaca safety monitor enabled");
    ALPACA_SAFEMON = true;
    saveHwPrefs();
}

void commandAlpacaSafemonOff() {
    logConsoleMessage("[CONSOLE] Alpaca safety monitor disabled");
    ALPACA_SAFEMON = false;
    saveHwPrefs();
}

void commandHwDs3231On() {
    logConsoleMessage("[CONSOLE] DS3231 enabled");
    HARDWARE_DS3231 = true;
    saveHwPrefs();
}

void commandHwDs3231Off() {
    logConsoleMessage("[CONSOLE] DS3231 disabled");
    HARDWARE_DS3231 = false;
    saveHwPrefs();
}

void commandHwBmp280On() {
    logConsoleMessage("[CONSOLE] BMP280 enabled");
    HARDWARE_BMP280 = true;
    saveHwPrefs();
}

void commandHwBmp280Off() {
    logConsoleMessage("[CONSOLE] BMP280 disabled");
    HARDWARE_BMP280 = false;
    saveHwPrefs();
}

void commandHwAht20On() {
    logConsoleMessage("[CONSOLE] AHT20 enabled");
    HARDWARE_AHT20 = true;
    saveHwPrefs();
}

void commandHwAht20Off() {
    logConsoleMessage("[CONSOLE] AHT20 disabled");
    HARDWARE_AHT20 = false;
    saveHwPrefs();
}

void commandHwSht45On() {
    logConsoleMessage("[CONSOLE] SHT45 enabled");
    HARDWARE_SHT45 = true;
    saveHwPrefs();
}

void commandHwSht45Off() {
    logConsoleMessage("[CONSOLE] SHT45 disabled");
    HARDWARE_SHT45 = false;
    saveHwPrefs();
}

void commandHwMlx90614On() {
    logConsoleMessage("[CONSOLE] MLX90614 enabled");
    HARDWARE_MLX90614 = true;
    saveHwPrefs();
}

void commandHwMlx90614Off() {
    logConsoleMessage("[CONSOLE] MLX90614 disabled");
    HARDWARE_MLX90614 = false;
    saveHwPrefs();
}

void commandHwTsl2591On() {
    logConsoleMessage("[CONSOLE] TSL2591 enabled");
    HARDWARE_TSL2591 = true;
    saveHwPrefs();
}

void commandHwTsl2591Off() {
    logConsoleMessage("[CONSOLE] TSL2591 disabled");
    HARDWARE_TSL2591 = false;
    saveHwPrefs();
}

void commandHwAnemo4403On() {
    logConsoleMessage("[CONSOLE] ANEMO4403 enabled");
    HARDWARE_ANEMO4403 = true;
    saveHwPrefs();
}

void commandHwAnemo4403Off() {
    logConsoleMessage("[CONSOLE] ANEMO4403 disabled");
    HARDWARE_ANEMO4403 = false;
    saveHwPrefs();
}

void commandHwUicpalOn() {
    logConsoleMessage("[CONSOLE] UICPAL enabled");
    HARDWARE_UICPAL = true;
    saveHwPrefs();
}

void commandHwUicpalOff() {
    logConsoleMessage("[CONSOLE] UICPAL disabled");
    HARDWARE_UICPAL = false;
    saveHwPrefs();
}

void commandHwRg15On() {
    logConsoleMessage("[CONSOLE] RG15 enabled");
    HARDWARE_RG15 = true;
    saveHwPrefs();
}

void commandHwRg15Off() {
    logConsoleMessage("[CONSOLE] RG15 disabled");
    HARDWARE_RG15 = false;
    saveHwPrefs();
}

void commandTempWeightBmp280(float weight) {
    T_WEIGHT_BMP280 = weight;
    saveThWeightsPrefs();
    // logConsoleMessage("[CONSOLE] BMP280 temperature calc weight " + String(T_NORM_WEIGHT_BMP280) + " [" + String(T_WEIGHT_BMP280) + "]");
}

void commandTempWeightAht20(float weight) {
    T_WEIGHT_AHT20 = weight;
    saveThWeightsPrefs();
    // logConsoleMessage("[CONSOLE] AHT20 temperature calc weight " + String(T_NORM_WEIGHT_AHT20) + " [" + String(T_WEIGHT_AHT20) + "]");
}

void commandTempWeightSht45(float weight) {
    T_WEIGHT_SHT45 = weight;
    saveThWeightsPrefs();
    // logConsoleMessage("[CONSOLE] SHT45 temperature calc weight " + String(T_NORM_WEIGHT_SHT45) + " [" + String(T_WEIGHT_SHT45) + "]");
}

void commandHumiWeightAht20(float weight) {
    H_WEIGHT_AHT20 = weight;
    saveThWeightsPrefs();
    // logConsoleMessage("[CONSOLE] AHT20 humidity calc weight " + String(H_NORM_WEIGHT_AHT20) + " [" + String(H_WEIGHT_AHT20) + "]");
}

void commandHumiWeightSht45(float weight) {
    H_WEIGHT_SHT45 = weight;
    saveThWeightsPrefs();
    // logConsoleMessage("[CONSOLE] SHT45 humidity calc weight " + String(H_NORM_WEIGHT_SHT45) + " [" + String(H_WEIGHT_SHT45) + "]");
}

void initConsoleCommands() {

    console_commands["help"] = commandHelp;
    console_commands["helpinfo"] = commandHelpInfo;
    console_commands["helpgeneral"] = commandHelpGeneral;
    console_commands["helplog"] = commandHelpLog;
    console_commands["helptarget"] = commandHelpTarget;
    console_commands["helphw"] = commandHelpHw;
    console_commands["helpalpaca"] = commandHelpAlpaca;
    console_commands["helptemp"] = commandHelpTemp;
    console_commands["helphumi"] = commandHelpHumi;

    console_commands["reboot"] = commandReboot;

    console_commands["cal"] = commandCalibrateState;
    console_commands["calibrate"] = commandCalibrateState;
    console_commands["calibration"] = commandCalibrateState;

    console_commands["target"] = commandTargetState;
    console_commands["targets"] = commandTargetState;

    console_commands["log"] = commandLogState;
    console_commands["logs"] = commandLogState;

    console_commands["hw"] = commandHardwareState;
    console_commands["hardware"] = commandHardwareState;

    console_commands["temp"] = commandTempWeightState;
    console_commands["temperature"] = commandTempWeightState;

    console_commands["humi"] = commandHumiWeightState;
    console_commands["humidity"] = commandHumiWeightState;

    console_commands["targetserialon"] = commandTargetSerialOn;
    console_commands["targetserialoff"] = commandTargetSerialOff;
    console_commands["targetconsoleon"] = commandTargetConsoleOn;
    console_commands["targetconsoleoff"] = commandTargetConsoleOff;
    console_commands["targetmqtton"] = commandTargetMqttOn;
    console_commands["targetmqttoff"] = commandTargetMqttOff;
    console_commands["targetledon"] = commandTargetLedOn;
    console_commands["targetledoff"] = commandTargetLedOff;

    console_commands["logmainon"] = commandLogMainOn;
    console_commands["logmainoff"] = commandLogMainOff;
    console_commands["logmeteoon"] = commandLogMeteoOn;
    console_commands["logmeteoslow"] = commandLogMeteoSlow;
    console_commands["logmeteooff"] = commandLogMeteoOff;
    console_commands["logalpacaon"] = commandLogAlpacaOn;
    console_commands["logalpacaoff"] = commandLogAlpacaOff;
    console_commands["logobsconon"] = commandLogObsconOn;
    console_commands["logobsconslow"] = commandLogObsconSlow;
    console_commands["logobsconoff"] = commandLogObsconOff;
    console_commands["logsafemonon"] = commandLogSafemonOn;
    console_commands["logsafemonslow"] = commandLogSafemonSlow;
    console_commands["logsafemonoff"] = commandLogSafemonOff;
    console_commands["logwifion"] = commandLogWifiOn;
    console_commands["logwifioff"] = commandLogWifiOff;
    console_commands["logotaon"] = commandLogOtaOn;
    console_commands["logotaoff"] = commandLogOtaOff;
    console_commands["logtechon"] = commandLogTechOn;
    console_commands["logtechoff"] = commandLogTechOff;
    console_commands["logon"] = commandLogOn;
    console_commands["logslow"] = commandLogSlow;
    console_commands["logoff"] = commandLogOff;

    console_commands["alpacaobsconon"] = commandAlpacaObsconOn;
    console_commands["alpacaobsconoff"] = commandAlpacaObsconOff;
    console_commands["alpacasafemonon"] = commandAlpacaSafemonOn;
    console_commands["alpacasafemonoff"] = commandAlpacaSafemonOff;

    console_commands["hwds3231on"] = commandHwDs3231On;
    console_commands["hwds3231off"] = commandHwDs3231Off;
    console_commands["hwbmp280on"] = commandHwBmp280On;
    console_commands["hwbmp280off"] = commandHwBmp280Off;
    console_commands["hwaht20on"] = commandHwAht20On;
    console_commands["hwaht20off"] = commandHwAht20Off;
    console_commands["hwsht45on"] = commandHwSht45On;
    console_commands["hwsht45off"] = commandHwSht45Off;
    console_commands["hwmlx90614on"] = commandHwMlx90614On;
    console_commands["hwmlx90614off"] = commandHwMlx90614Off;
    console_commands["hwtsl2591on"] = commandHwTsl2591On;
    console_commands["hwtsl2591off"] = commandHwTsl2591Off;
    console_commands["hwanemo4403on"] = commandHwAnemo4403On;
    console_commands["hwanemo4403off"] = commandHwAnemo4403Off;
    console_commands["hwuicpalon"] = commandHwUicpalOn;
    console_commands["hwuicpaloff"] = commandHwUicpalOff;
    console_commands["hwrg15on"] = commandHwRg15On;
    console_commands["hwrg15off"] = commandHwRg15Off;

    console_commands["uptime"] = commandUptime;
    console_commands["fault"] = commandFaults;
    console_commands["faults"] = commandFaults;
}

TempHumiWeightCommand parseTempHumiWeightCommand(const std::string &input) {
    TempHumiWeightCommand result = {"", {0}, 0, false};
    std::string str = input;
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    for (char &thwc : str) {
        if (thwc == ',' || thwc == ';' || thwc == '/') {
            thwc = ' ';
        }
    }
    std::istringstream iss(str);
    std::string w1, w2;
    if (!(iss >> w1 >> w2)) {
        return result;
    }
    std::string cmd = (w1.find("temp") == 0   ? "temp"
                       : w1.find("humi") == 0 ? "humi"
                                              : "");
    if (cmd.empty()) {
        return result;
    }
    if (w2.find("weight") == 0) {
        cmd += "weight";
    } else if (w2.find("sht") == 0) {
        cmd += "sht";
    } else if (w2.find("aht") == 0) {
        cmd += "aht";
    } else if (w2.find("bmp") == 0) {
        cmd += "bmp";
    } else {
        return result;
    }
    float v;
    while (iss >> v && result.valueCount < 3) {
        result.values[result.valueCount++] = v;
    }
    int expected = (cmd == "tempweight"   ? 3
                    : cmd == "humiweight" ? 2
                                          : 1);
    if (result.valueCount != expected) {
        return result;
    }
    result.command = cmd;
    result.success = true;
    return result;
}

CalibrateCommand parseCalibrateCommand(const std::string &input) {
    // Все 18 валидных команд (на выходе]):
    // calbmptemp
    // calbmppres
    // calahttemp
    // calahthumi
    // calshttemp
    // calshthumi
    // caldewpoint
    // calmlxamb
    // calmlxobj
    // calmlxsky
    // calmlxcloud
    // caltslbright
    // caltslsky
    // caltslsqm
    // calanemowindspeed
    // calanemowindgust
    // caluicpalrain
    // calrgrain
    CalibrateCommand result = {"", 0.0, 0.0, false};
    std::string str = input;
    // Trim
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return result;
    }
    str = str.substr(start, end - start + 1);
    // To lowercase
    for (char &c : str) {
        if (c >= 'A' && c <= 'Z')
            c = c + 32;
    }
    // Remove extra spaces
    size_t pos = 0;
    while ((pos = str.find("  ", pos)) != std::string::npos) {
        str.erase(pos, 1);
    }
    if (str.find("cal") != 0) {
        return result;
    }
    std::string tokens[10];
    int count = 0;
    size_t i = 0;
    size_t tokenStart = 0;
    while (i <= str.length()) {
        if (i == str.length() || str[i] == ' ') {
            if (i > tokenStart) {
                tokens[count++] = str.substr(tokenStart, i - tokenStart);
                if (count >= 10) {
                    break;
                }
            }
            tokenStart = i + 1;
        }
        i++;
    }
    if (count < 3) {
        return result;
    }
    std::string cmd = "";
    int floatStart = -1;
    // BMP280
    if (tokens[1].find("bmp") == 0) {
        if (tokens[2].find("temp") == 0) {
            cmd = "calbmptemp";
            floatStart = 3;
        } else if (tokens[2].find("pres") == 0) {
            cmd = "calbmppres";
            floatStart = 3;
        }
    }
    // AHT20
    else if (tokens[1].find("aht") == 0) {
        if (tokens[2].find("temp") == 0) {
            cmd = "calahttemp";
            floatStart = 3;
        } else if (tokens[2].find("humi") == 0) {
            cmd = "calahthumi";
            floatStart = 3;
        }
    }
    // SHT45
    else if (tokens[1].find("sht") == 0) {
        if (tokens[2].find("temp") == 0) {
            cmd = "calshttemp";
            floatStart = 3;
        } else if (tokens[2].find("humi") == 0) {
            cmd = "calshthumi";
            floatStart = 3;
        }
    }
    // Dew Point
    else if (tokens[1].find("dew") == 0 && count >= 4) {
        if (tokens[2].find("point") == 0) {
            cmd = "caldewpoint";
            floatStart = 3;
        }
    }
    // MLX90614
    else if (tokens[1].find("mlx") == 0) {
        if (tokens[2].find("amb") == 0) {
            cmd = "calmlxamb";
            floatStart = 3;
        } else if (tokens[2].find("obj") == 0) {
            cmd = "calmlxobj";
            floatStart = 3;
        } else if (tokens[2].find("sky") == 0) {
            if (count >= 4 && tokens[3].find("t") == 0) {
                cmd = "calmlxsky";
                floatStart = 4;
            } else {
                cmd = "calmlxsky";
                floatStart = 3;
            }
        } else if (tokens[2].find("cloud") == 0) {
            if (count >= 4 && tokens[3].find("c") == 0) {
                cmd = "calmlxcloud";
                floatStart = 4;
            } else {
                cmd = "calmlxcloud";
                floatStart = 3;
            }
        }
    }
    // TSL2591
    else if (tokens[1].find("tsl") == 0) {
        if (tokens[2].find("bright") == 0) {
            cmd = "caltslbright";
            floatStart = 3;
        } else if (tokens[2].find("sky") == 0) {
            cmd = "caltslsky";
            floatStart = 3;
        } else if (tokens[2].find("sqm") == 0) {
            cmd = "caltslsqm";
            floatStart = 3;
        }
    }
    // ANEMO4403
    else if (tokens[1].find("anem") == 0) {
        if (tokens[2].find("wind") == 0) {
            if (count >= 4 && tokens[3].find("s") == 0) {
                cmd = "calanemowindspeed";
                floatStart = 4;
            } else if (count >= 4 && tokens[3].find("g") == 0) {
                cmd = "calanemowindgust";
                floatStart = 4;
            }
        } else if (tokens[2].find("winds") == 0) {
            cmd = "calanemowindspeed";
            floatStart = 3;
        } else if (tokens[2].find("windg") == 0) {
            cmd = "calanemowindgust";
            floatStart = 3;
        }
    }
    // UICPAL
    else if (tokens[1].find("uic") == 0) {
        if (tokens[2].find("rain") == 0) {
            if (count >= 4 && tokens[3].find("r") == 0) {
                cmd = "caluicpalrain";
                floatStart = 4;
            } else {
                cmd = "caluicpalrain";
                floatStart = 3;
            }
        }
    }
    // RG15
    else if (tokens[1].find("rg") == 0) {
        if (tokens[2].find("rain") == 0) {
            if (count >= 4 && tokens[3].find("r") == 0) {
                cmd = "calrgrain";
                floatStart = 4;
            } else {
                cmd = "calrgrain";
                floatStart = 3;
            }
        }
    }
    if (cmd.empty() || floatStart < 0 || count < floatStart + 2) {
        return result;
    }
    result.a = std::atof(tokens[floatStart].c_str());
    result.b = std::atof(tokens[floatStart + 1].c_str());
    result.command = cmd;
    result.success = true;
    return result;
}

void processConsoleCommand(const std::string &msg) {
    // Calibrate commands
    CalibrateCommand calc = parseCalibrateCommand(msg);
    if (calc.success) {
        if (calc.command == "calbmptemp") {
            commandCalibrateBMP280Temperature(calc.a, calc.b);
        }
        if (calc.command == "calbmppres") {
            commandCalibrateBMP280Pressure(calc.a, calc.b);
        }
        if (calc.command == "calahttemp") {
            commandCalibrateAHT20Temperature(calc.a, calc.b);
        }
        if (calc.command == "calahthumi") {
            commandCalibrateAHT20Humidity(calc.a, calc.b);
        }
        if (calc.command == "calshttemp") {
            commandCalibrateSHT45Temperature(calc.a, calc.b);
        }
        if (calc.command == "calshthumi") {
            commandCalibrateSHT45Humidity(calc.a, calc.b);
        }
        if (calc.command == "caldewpoint") {
            commandCalibrateDewPoint(calc.a, calc.b);
        }
        if (calc.command == "calmlxamb") {
            commandCalibrateMLX90614Ambient(calc.a, calc.b);
        }
        if (calc.command == "calmlxobj") {
            commandCalibrateMLX90614Object(calc.a, calc.b);
        }
        if (calc.command == "calmlxsky") {
            commandCalibrateMLX90614SkyTemperature(calc.a, calc.b);
        }
        if (calc.command == "calmlxcloud") {
            commandCalibrateMLX90614CloudCover(calc.a, calc.b);
        }
        if (calc.command == "caltslbright") {
            commandCalibrateTSL2591SkyBrightness(calc.a, calc.b);
        }
        if (calc.command == "caltslsky" || calc.command == "caltslsqm") {
            commandCalibrateTSL2591SkyQuality(calc.a, calc.b);
        }
        if (calc.command == "calanemowindspeed") {
            commandCalibrateANEMO4403WindSpeed(calc.a, calc.b);
        }
        if (calc.command == "calanemowindgust") {
            commandCalibrateANEMO4403WindGust(calc.a, calc.b);
        }
        if (calc.command == "caluicpalrain") {
            commandCalibrateUICPALRainRate(calc.a, calc.b);
        }
        if (calc.command == "calrgrain") {
            commandCalibrateRG15RainRate(calc.a, calc.b);
        }
        commandCalibrateState();
        return;
    }
    // Temp and humi calc weights commmands
    TempHumiWeightCommand thwc = parseTempHumiWeightCommand(msg);
    if (thwc.success) {
        if (thwc.command == "tempweight") {
            commandTempWeightSht45(thwc.values[0]);
            commandTempWeightAht20(thwc.values[1]);
            commandTempWeightBmp280(thwc.values[2]);
            commandTempWeightState();
        }
        if (thwc.command == "tempsht") {
            commandTempWeightSht45(thwc.values[0]);
            commandTempWeightState();
        }
        if (thwc.command == "tempaht") {
            commandTempWeightAht20(thwc.values[0]);
            commandTempWeightState();
        }
        if (thwc.command == "tempbmp") {
            commandTempWeightBmp280(thwc.values[0]);
            commandTempWeightState();
        }
        if (thwc.command == "humiweight") {
            commandHumiWeightSht45(thwc.values[0]);
            commandHumiWeightAht20(thwc.values[1]);
            commandHumiWeightState();
        }
        if (thwc.command == "humisht") {
            commandHumiWeightSht45(thwc.values[0]);
            commandHumiWeightState();
        }
        if (thwc.command == "humiaht") {
            commandHumiWeightAht20(thwc.values[0]);
            commandHumiWeightState();
        }
        return;
    }
    // Slow log commands
    std::string cmd;
    cmd.resize(msg.size());
    std::transform(msg.begin(), msg.end(), cmd.begin(),
                   [](unsigned char thwc) { return std::tolower(thwc); });
    cmd.erase(remove_if(cmd.begin(), cmd.end(), isspace), cmd.end());

    if (cmd.length() > 12 && cmd.substr(0, 12) == "logmeteoslow") {
        commandLogMeteoSlowDelay(static_cast<uint16_t>(std::stoul(cmd.substr(12))));
        return;
    }
    if (cmd.length() > 13 && cmd.substr(0, 13) == "logobsconslow") {
        commandLogObsconSlowDelay(static_cast<uint16_t>(std::stoul(cmd.substr(13))));
        return;
    }
    if (cmd.length() > 14 && cmd.substr(0, 14) == "logsafemonslow") {
        commandLogSafemonSlowDelay(static_cast<uint16_t>(std::stoul(cmd.substr(14))));
        return;
    }
    // Commands
    auto it = console_commands.find(cmd);
    if (it != console_commands.end()) {
        it->second(); // Execute the associated function
    } else {
        logConsoleMessage("[CONSOLE] Command not found, use command \"help\" please");
    }
}
