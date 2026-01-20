#include "console.h"
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
    logConsoleMessage("[HELP] Available console commands");
    logConsoleMessage("[HELP] --------------------------");
    logConsoleMessage("[HELP] Info:");
    logConsoleMessage("[HELP]   help   - this screen");
    logConsoleMessage("[HELP]   log    - show curent log settings");
    logConsoleMessage("[HELP]   target - show curent log target settings");
    logConsoleMessage("[HELP]   hw     - show curent log settings");
    logConsoleMessage("[HELP]   temp   - show curent temperature calc weights");
    logConsoleMessage("[HELP]   humi   - show curent humidity calc weights");
    logConsoleMessage("[HELP] General:");
    logConsoleMessage("[HELP]   reboot            - restart esp32 ascom alpaca device");
    logConsoleMessage("[HELP] Log settings:");
    logConsoleMessage("[HELP]   log on/off                    - enable/disable all logging");
    logConsoleMessage("[HELP]   log main on/off               - enable/disable main logging");
    logConsoleMessage("[HELP]   log alpaca on/off             - enable/disable alpaca server logging");
    logConsoleMessage("[HELP]   log meteo on/slow [nnn]/off   - enable/slow, nnn seconds/disable meteo logging");
    logConsoleMessage("[HELP]   log obscon on/slow [nnn]/off  - enable/slow, nnn seconds/disable observing conditions logging");
    logConsoleMessage("[HELP]   log safemon on/slow [nnn]/off - enable/slow, nnn seconds/disable safety monitor logging");
    logConsoleMessage("[HELP]   log wifi on/off               - enable/disable wifi manager logging");
    logConsoleMessage("[HELP]   log ota on/off                - enable/disable ota update logging");
    logConsoleMessage("[HELP]   log tech on/off               - enable/disable tech sensor data logging");
    logConsoleMessage("[HELP] Log target settings:");
    logConsoleMessage("[HELP]   target serial on/off  - enable/disable serial log output");
    logConsoleMessage("[HELP]   target console on/off - enable/disable console log output");
    logConsoleMessage("[HELP]   target mqtt on/off    - enable/disable mqtt log output");
    logConsoleMessage("[HELP]   target led on/off     - enable/disable led log output");
    logConsoleMessage("[HELP] Hardware settings (reboot required):");
    logConsoleMessage("[HELP]   hw bmp280 on/off    - enable/disable BMP280 sensor");
    logConsoleMessage("[HELP]   hw aht20 on/off     - enable/disable AHT20 sensor");
    logConsoleMessage("[HELP]   hw sht45 on/off     - enable/disable SHT45 sensor");
    logConsoleMessage("[HELP]   hw mlx90614 on/off  - enable/disable MLX90614 sensor");
    logConsoleMessage("[HELP]   hw tsl2591 on/off   - enable/disable TSL2591 sensor");
    logConsoleMessage("[HELP]   hw anemo4403 on/off - enable/disable ANEMO4403 sensor");
    logConsoleMessage("[HELP]   hw uicpal on/off    - enable/disable UICPAL sensor");
    logConsoleMessage("[HELP]   hw rg15 on/off      - enable/disable RG-15 sensor");
    logConsoleMessage("[HELP] Temperature calc weights:");
    logConsoleMessage("[HELP]   temp sht n.nn - set SHT45 temperature calc weight");
    logConsoleMessage("[HELP]   temp aht n.nn - set AHT20 temperature calc weight");
    logConsoleMessage("[HELP]   temp bmp n.nn - set BMP280 temperature calc weight");
    logConsoleMessage("[HELP]   temp weight n.nn n.nn n.nn - set SHT45/AHT20/BMP280 temperature calc weights at once");
    logConsoleMessage("[HELP] Humidity calc weights:");
    logConsoleMessage("[HELP]   humi sht n.nn - set SHT45 humidity calc weight");
    logConsoleMessage("[HELP]   humi aht n.nn - set AHT20 humidity calc weight");
    logConsoleMessage("[HELP]   humi weight n.nn n.nn - set SHT45/AHT20 humidity calc weights at once");
    logConsoleMessage("[HELP] Alpaca settings (reboot required):");
    logConsoleMessage("[HELP]   alpaca obscon on/off  - enable/disable observing conditions service");
    logConsoleMessage("[HELP]   alpaca safemon on/off - enable/disable safety monitor service");
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
    logConsoleMessage("[INFO]   BMP280    - " + String(HARDWARE_BMP280 ? "enabled " : "disabled") + String(HARDWARE_BMP280 && INITED_BMP280 ? ", OK" : ", ERROR") + " (temperature and pressure)");
    logConsoleMessage("[INFO]   AHT20     - " + String(HARDWARE_AHT20 ? "enabled " : "disabled") + String(HARDWARE_AHT20 && INITED_AHT20 ? ", OK" : ", ERROR") + " (temperature and humidity)");
    logConsoleMessage("[INFO]   SHT45     - " + String(HARDWARE_SHT45 ? "enabled " : "disabled") + String(HARDWARE_SHT45 && INITED_SHT45 ? ", OK" : ", ERROR") + " (temperature and humidity)");
    logConsoleMessage("[INFO]   MLX90614  - " + String(HARDWARE_MLX90614 ? "enabled " : "disabled") + String(HARDWARE_MLX90614 && INITED_MLX90614 ? ", OK" : ", ERROR") + " (sky temperature)");
    logConsoleMessage("[INFO]   TSL2591   - " + String(HARDWARE_TSL2591 ? "enabled " : "disabled") + String(HARDWARE_TSL2591 && INITED_TSL2591 ? ", OK" : ", ERROR") + " (sky brightness)");
    logConsoleMessage("[INFO]   ANEMO4403 - " + String(HARDWARE_ANEMO4403 ? "enabled " : "disabled") + String(HARDWARE_ANEMO4403 && INITED_ANEMO4403 ? ", OK" : ", ERROR") + " (wind speed)");
    logConsoleMessage("[INFO]   UICPAL    - " + String(HARDWARE_UICPAL ? "enabled " : "disabled") + String(HARDWARE_UICPAL && INITED_UICPAL ? ", OK" : ", ERROR") + " (rain/snow sensor)");
    logConsoleMessage("[INFO]   RG15      - " + String(HARDWARE_RG15 ? "enabled " : "disabled") + String(HARDWARE_RG15 && INITED_RG15 ? ", OK" : ", ERROR") + " (rain rate sensor)");
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

void commandReboot() {
    logConsoleMessage("[CONSOLE] Immediate reboot requested!");
    logConsoleMessage("[REBOOT]");
    delay(1000);
    ESP.restart();
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
    console_commands["reboot"] = commandReboot;

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
}

TempHumiWeightCommand parseTempHumiWeightCommand(const std::string &input) {
    TempHumiWeightCommand result = {"", {0}, 0, false};
    std::string str = input;
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    for (char &c : str) {
        if (c == ',' || c == ';' || c == '/') {
            c = ' ';
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

void processConsoleCommand(const std::string &msg) {
    // Temp and humi calc weights commmands
    TempHumiWeightCommand c = parseTempHumiWeightCommand(msg);
    if (c.success) {
        if (c.command == "tempweight") {
            commandTempWeightSht45(c.values[0]);
            commandTempWeightAht20(c.values[1]);
            commandTempWeightBmp280(c.values[2]);
            commandTempWeightState();
        }
        if (c.command == "tempsht") {
            commandTempWeightSht45(c.values[0]);
            commandTempWeightState();
        }
        if (c.command == "tempaht") {
            commandTempWeightAht20(c.values[0]);
            commandTempWeightState();
        }
        if (c.command == "tempbmp") {
            commandTempWeightBmp280(c.values[0]);
            commandTempWeightState();
        }
        if (c.command == "humiweight") {
            commandHumiWeightSht45(c.values[0]);
            commandHumiWeightAht20(c.values[1]);
            commandHumiWeightState();
        }
        if (c.command == "humisht") {
            commandHumiWeightSht45(c.values[0]);
            commandHumiWeightState();
        }
        if (c.command == "humiaht") {
            commandHumiWeightAht20(c.values[0]);
            commandHumiWeightState();
        }
        return;
    }
    // Slow log commands
    std::string cmd;
    cmd.resize(msg.size());
    std::transform(msg.begin(), msg.end(), cmd.begin(),
                   [](unsigned char c) { return std::tolower(c); });
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
