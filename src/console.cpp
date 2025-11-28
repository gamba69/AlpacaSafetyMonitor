#include "console.h"
#include "hardware.h"
#include "log.h"
#include <Arduino.h>
#include <iterator>
#include <map>

std::map<std::string, std::function<void()>> console_commands;

void commandHelp() {
    logMessageConsole("[HELP] Available console commands");
    logMessageConsole("[HELP] --------------------------");
    logMessageConsole("[HELP] Info:");
    logMessageConsole("[HELP]   help - this screen");
    logMessageConsole("[HELP]   log  - show curent log settingd");
    logMessageConsole("[HELP]   hw   - show curent log settingd");
    logMessageConsole("[HELP] General:");
    logMessageConsole("[HELP]   reboot            - restart esp32 ascom alpaca device");
    logMessageConsole("[HELP] Log settings:");
    logMessageConsole("[HELP]   log on/off                  - enable/disable all logging");
    logMessageConsole("[HELP]   log main on/off             - enable/disable main logging");
    logMessageConsole("[HELP]   log alpaca on/off           - enable/disable alpaca server logging");
    logMessageConsole("[HELP]   log meteo on/slow [nnn]/off - enable/slow, nnn seconds/disable meteo logging");
    logMessageConsole("[HELP]   log oc on/slow [nnn]/off    - enable/slow, nnn seconds/disable observing conditions logging");
    logMessageConsole("[HELP]   log sm on/slow [nnn]/off    - enable/slow, nnn seconds/disable safety monitor logging");
    logMessageConsole("[HELP]   log wifi on/off             - enable/disable wifi manager logging");
    logMessageConsole("[HELP]   log ota on/off              - enable/disable ota update logging");
    logMessageConsole("[HELP] Hardware settings (reboot required):");
    logMessageConsole("[HELP]   hw bmp280 on/off   - enable/disable BMP280 sensor");
    logMessageConsole("[HELP]   hw aht20 on/off    - enable/disable AHT20 sensor");
    logMessageConsole("[HELP]   hw mlx90614 on/off - enable/disable MLX90614 sensor");
    logMessageConsole("[HELP]   hw tsl2591 on/off  - enable/disable TSL2591 sensor");
    logMessageConsole("[HELP]   hw uicpal on/off   - enable/disable UICPAL sensor");
    logMessageConsole("[HELP]   hw rg15 on/off     - enable/disable RG-15 sensor");
    logMessageConsole("[HELP] Alpaca settings (reboot required):");
    logMessageConsole("[HELP]   alpaca oc on/off   - enable/disable observing conditions service");
    logMessageConsole("[HELP]   alpaca sm on/off   - enable/disable safety monitor service");
}

void commandLogState() {
    logMessageConsole("[INFO] Logging state");
    logMessageConsole("[INFO] -------------");
    logMessageConsole("[INFO]  console - on");
    logMessageConsole("[INFO]  main    - " + String(logEnabled[LogMain] ? "on" : "off"));
    logMessageConsole("[INFO]  alpaca  - " + String(logEnabled[LogAlpaca] ? "on" : "off"));
    logMessageConsole("[INFO]  meteo   - " + String(logEnabled[LogMeteo] ? (logEnabled[LogMeteo] == LogOn ? "on" : ("slow " + String(logSlow[LogMeteo]))) : "off") + " [meteo sensors]");
    logMessageConsole("[INFO]  oc      - " + String(logEnabled[LogObservingConditions] ? (logEnabled[LogObservingConditions] == LogOn ? "ob" : ("slow " + String(logSlow[LogObservingConditions]))) : "off") + " [observing conditions]");
    logMessageConsole("[INFO]  sm      - " + String(logEnabled[LogSafetyMonitor] ? (logEnabled[LogSafetyMonitor] == LogOn ? "on" : ("slow " + String(logSlow[LogSafetyMonitor]))) : "off") + " [safety monitor]");
    logMessageConsole("[INFO]  wifi    - " + String(logEnabled[LogWifi] ? "on" : "off") + " [wifi manager]");
    logMessageConsole("[INFO]  ota     - " + String(logEnabled[LogOta] ? "on" : "off") + " [update manager]");
}

void commandHardwareState() {
    logMessageConsole("[INFO] Firmware supported hardware");
    logMessageConsole("[INFO] ---------------------------");
    logMessageConsole("[INFO] Sensors:");
    logMessageConsole("[INFO]   DS3231   - " + String(hwEnabled[hwDs3231] ? "enabled " : "disabled") + " (realtime clock)");
    logMessageConsole("[INFO]   BMP280   - " + String(hwEnabled[hwBmp280] ? "enabled " : "disabled") + " (temperature and pressure)");
    logMessageConsole("[INFO]   AHT20    - " + String(hwEnabled[hwAht20] ? "enabled " : "disabled") + " (temperature and humidity)");
    logMessageConsole("[INFO]   MLX90614 - " + String(hwEnabled[hwMlx90614] ? "enabled " : "disabled") + " (sky temperature)");
    logMessageConsole("[INFO]   TSL2591  - " + String(hwEnabled[hwTsl2591] ? "enabled " : "disabled") + " (sky brightness)");
    logMessageConsole("[INFO]   UICPAL   - " + String(hwEnabled[hwUicpal] ? "enabled " : "disabled") + " (rain/snow sensor)");
    logMessageConsole("[INFO]   RG15     - " + String(hwEnabled[hwRg15] ? "enabled " : "disabled") + " (rain rate sensor)");
    logMessageConsole("[INFO] Alpaca:");
    logMessageConsole("[INFO]   Observing conditions - " + String(hwEnabled[alpacaOc] ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Safety monitor       - " + String(hwEnabled[alpacaSm] ? "enabled" : "disabled"));
    logMessageConsole("[INFO] Obsering conditions:");
    logMessageConsole("[INFO]   Rain rate       - " + String(hwEnabled[ocRainRate] ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Temperature     - " + String(hwEnabled[ocTemperature] ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Humidity        - " + String(hwEnabled[ocHumidity] ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Dew point       - " + String(hwEnabled[ocDewPoint] ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Pressure        - " + String(hwEnabled[ocPressure] ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Sky temperature - " + String(hwEnabled[ocSkyTemp] ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Cloud cover     - " + String(hwEnabled[ocCloudCover] ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Star FWHM       - " + String(hwEnabled[ocFwhm] ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Sky brightness  - " + String(hwEnabled[ocSkyBrightness] ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Sky quality     - " + String(hwEnabled[ocSkyQuality] ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Wind direction  - " + String(hwEnabled[ocWindDirection] ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Wind speed      - " + String(hwEnabled[ocWindSpeed] ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Wind gust       - " + String(hwEnabled[ocWindGust] ? "enabled" : "disabled"));
    logMessageConsole("[INFO] Safety monitor:");
    logMessageConsole("[INFO]   Rain rate       - " + String(hwEnabled[smRainRate] ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Temperature     - " + String(hwEnabled[smTemperature] ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Humidity        - " + String(hwEnabled[smHumidity] ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Dew point       - " + String(hwEnabled[smDewPoint] ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Sky temperature - " + String(hwEnabled[smSkyTemp] ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Wind speed      - " + String(hwEnabled[smWindSpeed] ? "enabled" : "disabled"));
}

void commandReboot() {
    logMessageConsole("[CONSOLE] Immediate reboot requested!");
    logMessageConsole("[REBOOT]");
    delay(1000);
    ESP.restart();
}

void commandLogMainOn() {
    logMessageConsole("[CONSOLE] Main logging enabled");
    logEnabled[LogMain] = LogOn;
    saveLogPrefs();
}

void commandLogMainOff() {
    logMessageConsole("[CONSOLE] Main logging disabled");
    logEnabled[LogMain] = LogOff;
    saveLogPrefs();
}

void commandLogMeteoOn() {
    logMessageConsole("[CONSOLE] Meteo logging enabled");
    logEnabled[LogMeteo] = LogOn;
    saveLogPrefs();
}

void commandLogMeteoOff() {
    logMessageConsole("[CONSOLE] Meteo logging disabled");
    logEnabled[LogMeteo] = LogOff;
    saveLogPrefs();
}

void commandLogMeteoSlow() {
    logMessageConsole("[CONSOLE] Meteo logging slow " + logSlow[LogMeteo]);
    logEnabled[LogMeteo] = LogSlow;
    saveLogPrefs();
}

void commandLogMeteoSlowDelay(uint16_t delay) {
    logMessageConsole("[CONSOLE] Meteo logging slow " + String(delay));
    logEnabled[LogMeteo] = LogSlow;
    logSlow[LogMeteo] = delay;
    saveLogPrefs();
}

void commandLogAlpacaOn() {
    logMessageConsole("[CONSOLE] Alpaca logging enabled");
    logEnabled[LogAlpaca] = LogOn;
    saveLogPrefs();
}

void commandLogAlpacaOff() {
    logMessageConsole("[CONSOLE] Alpaca logging disabled");
    logEnabled[LogAlpaca] = LogOff;
    saveLogPrefs();
}

void commandLogOcOn() {
    logMessageConsole("[CONSOLE] Observing conditions logging enabled");
    logEnabled[LogObservingConditions] = LogOn;
    saveLogPrefs();
}

void commandLogOcOff() {
    logMessageConsole("[CONSOLE] Observing conditions logging disabled");
    logEnabled[LogObservingConditions] = LogOff;
    saveLogPrefs();
}

void commandLogOcSlow() {
    logMessageConsole("[CONSOLE] Observing conditions logging slow " + String(logSlow[LogObservingConditions]));
    logEnabled[LogObservingConditions] = LogSlow;
    saveLogPrefs();
}

void commandLogOcSlowDelay(uint16_t delay) {
    logMessageConsole("[CONSOLE] Observing conditions logging slow " + String(delay));
    logEnabled[LogObservingConditions] = LogSlow;
    logSlow[LogObservingConditions] = delay;
    saveLogPrefs();
}

void commandLogSmOn() {
    logMessageConsole("[CONSOLE] Safety monitor logging enabled");
    logEnabled[LogSafetyMonitor] = LogOn;
    saveLogPrefs();
}

void commandLogSmOff() {
    logMessageConsole("[CONSOLE] Safety monitor logging disabled");
    logEnabled[LogSafetyMonitor] = LogOff;
    saveLogPrefs();
}

void commandLogSmSlow() {
    logMessageConsole("[CONSOLE] Safety monitor logging slow " + String(logSlow[LogSafetyMonitor]));
    logEnabled[LogSafetyMonitor] = LogSlow;
    saveLogPrefs();
}

void commandLogSmSlowDelay(uint16_t delay) {
    logMessageConsole("[CONSOLE] Safety monitor logging slow " + String(delay));
    logEnabled[LogSafetyMonitor] = LogSlow;
    logSlow[LogSafetyMonitor] = delay;
    saveLogPrefs();
}

void commandLogWifiOn() {
    logMessageConsole("[CONSOLE] Wifi monitor logging enabled");
    logEnabled[LogWifi] = LogOn;
    saveLogPrefs();
}

void commandLogWifiOff() {
    logMessageConsole("[CONSOLE] Wifi logging disabled");
    logEnabled[LogWifi] = LogOff;
    saveLogPrefs();
}

void commandLogOtaOn() {
    logMessageConsole("[CONSOLE] Ota monitor logging enabled");
    logEnabled[LogOta] = LogOn;
    saveLogPrefs();
}

void commandLogOtaOff() {
    logMessageConsole("[CONSOLE] Ota logging disabled");
    logEnabled[LogOta] = LogOff;
    saveLogPrefs();
}

void commandLogOn() {
    logMessageConsole("[CONSOLE] All logging enabled");
    std::fill(std::begin(logEnabled), std::end(logEnabled), LogOn);
    saveLogPrefs();
}

void commandLogOff() {
    logMessageConsole("[CONSOLE] All logging disabled");
    std::fill(std::begin(logEnabled), std::end(logEnabled), LogOff);
    saveLogPrefs();
}

void commandLogSlow() {
    logMessageConsole("[CONSOLE] All logging enabled, slow");
    std::fill(std::begin(logEnabled), std::end(logEnabled), LogOn);
    logEnabled[LogMeteo] = LogSlow;
    logEnabled[LogObservingConditions] = LogSlow;
    logEnabled[LogSafetyMonitor] = LogSlow;
    saveLogPrefs();
}

void commandAlpacaOcOn() {
    logMessageConsole("[CONSOLE] Alpaca observing conditions enabled");
    hwEnabled[alpacaOc] = true;
    saveHwPrefs();
}

void commandAlpacaOcOff() {
    logMessageConsole("[CONSOLE] Alpaca observing conditions disabled");
    hwEnabled[alpacaOc] = false;
    saveHwPrefs();
}

void commandAlpacaSmOn() {
    logMessageConsole("[CONSOLE] Alpaca safety monitor enabled");
    hwEnabled[alpacaSm] = true;
    saveHwPrefs();
}

void commandAlpacaSmOff() {
    logMessageConsole("[CONSOLE] Alpaca safety monitor disabled");
    hwEnabled[alpacaSm] = false;
    saveHwPrefs();
}

void commandHwDs3231On() {
    logMessageConsole("[CONSOLE] DS3231 enabled");
    hwEnabled[hwDs3231] = true;
    saveHwPrefs();
}

void commandHwDs3231Off() {
    logMessageConsole("[CONSOLE] DS3231 disabled");
    hwEnabled[hwDs3231] = false;
    saveHwPrefs();
}

void commandHwBmp280On() {
    logMessageConsole("[CONSOLE] BMP280 enabled");
    hwEnabled[hwBmp280] = true;
    saveHwPrefs();
}

void commandHwBmp280Off() {
    logMessageConsole("[CONSOLE] BMP280 disabled");
    hwEnabled[hwBmp280] = false;
    saveHwPrefs();
}

void commandHwAht20On() {
    logMessageConsole("[CONSOLE] AHT20 enabled");
    hwEnabled[hwAht20] = true;
    saveHwPrefs();
}

void commandHwAht20Off() {
    logMessageConsole("[CONSOLE] AHT20 disabled");
    hwEnabled[hwAht20] = false;
    saveHwPrefs();
}

void commandHwMlx90614On() {
    logMessageConsole("[CONSOLE] MLX90614 enabled");
    hwEnabled[hwMlx90614] = true;
    saveHwPrefs();
}

void commandHwMlx90614Off() {
    logMessageConsole("[CONSOLE] MLX90614 disabled");
    hwEnabled[hwMlx90614] = false;
    saveHwPrefs();
}

void commandHwTsl2591On() {
    logMessageConsole("[CONSOLE] TSL2591 enabled");
    hwEnabled[hwTsl2591] = true;
    saveHwPrefs();
}

void commandHwTsl2591Off() {
    logMessageConsole("[CONSOLE] TSL2591 disabled");
    hwEnabled[hwTsl2591] = false;
    saveHwPrefs();
}

void commandHwUicpalOn() {
    logMessageConsole("[CONSOLE] UICPAL enabled");
    hwEnabled[hwUicpal] = true;
    saveHwPrefs();
}

void commandHwUicpalOff() {
    logMessageConsole("[CONSOLE] UICPAL disabled");
    hwEnabled[hwUicpal] = false;
    saveHwPrefs();
}

void commandHwRg15On() {
    logMessageConsole("[CONSOLE] RG15 enabled");
    hwEnabled[hwRg15] = true;
    saveHwPrefs();
}

void commandHwRg15Off() {
    logMessageConsole("[CONSOLE] RG15 disabled");
    hwEnabled[hwRg15] = false;
    saveHwPrefs();
}

void initConsoleCommands() {

    console_commands["help"] = commandHelp;
    console_commands["reboot"] = commandReboot;

    console_commands["log"] = commandLogState;
    console_commands["logs"] = commandLogState;

    console_commands["hw"] = commandHardwareState;
    console_commands["hardware"] = commandHardwareState;

    console_commands["logmainon"] = commandLogMainOn;
    console_commands["logmainoff"] = commandLogMainOff;
    console_commands["logmeteoon"] = commandLogMeteoOn;
    console_commands["logmeteoslow"] = commandLogMeteoSlow;
    console_commands["logmeteooff"] = commandLogMeteoOff;
    console_commands["logalpacaon"] = commandLogAlpacaOn;
    console_commands["logalpacaoff"] = commandLogAlpacaOff;
    console_commands["logocon"] = commandLogOcOn;
    console_commands["logocslow"] = commandLogOcSlow;
    console_commands["logocoff"] = commandLogOcOff;
    console_commands["logsmon"] = commandLogSmOn;
    console_commands["logsmslow"] = commandLogSmSlow;
    console_commands["logsmoff"] = commandLogSmOff;
    console_commands["logwifion"] = commandLogWifiOn;
    console_commands["logwifioff"] = commandLogWifiOff;
    console_commands["logotaon"] = commandLogOtaOn;
    console_commands["logotaoff"] = commandLogOtaOff;
    console_commands["logon"] = commandLogOn;
    console_commands["logslow"] = commandLogSlow;
    console_commands["logoff"] = commandLogOff;

    console_commands["alpacaocon"] = commandAlpacaOcOn;
    console_commands["alpacaocoff"] = commandAlpacaOcOff;
    console_commands["alpacasmon"] = commandAlpacaSmOn;
    console_commands["alpacasmoff"] = commandAlpacaSmOff;

    console_commands["hwds3231on"] = commandHwDs3231On;
    console_commands["hwds3231off"] = commandHwDs3231Off;
    console_commands["hwbmp280on"] = commandHwBmp280On;
    console_commands["hwbmp280off"] = commandHwBmp280Off;
    console_commands["hwaht20on"] = commandHwAht20On;
    console_commands["hwaht20off"] = commandHwAht20Off;
    console_commands["hwmlx90614on"] = commandHwMlx90614On;
    console_commands["hwmlx90614off"] = commandHwMlx90614Off;
    console_commands["hwtsl2591on"] = commandHwTsl2591On;
    console_commands["hwtsl2591off"] = commandHwTsl2591Off;
    console_commands["hwuicpalon"] = commandHwUicpalOn;
    console_commands["hwuicpaloff"] = commandHwUicpalOff;
    console_commands["hwrg15on"] = commandHwRg15On;
    console_commands["hwrg15off"] = commandHwRg15Off;
}

void IRAM_ATTR processConsoleCommand(const std::string &msg) {
    std::string cmd;
    cmd.resize(msg.size());
    std::transform(msg.begin(), msg.end(), cmd.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    cmd.erase(remove_if(cmd.begin(), cmd.end(), isspace), cmd.end());

    if (cmd.length() > 12 && cmd.substr(0, 12) == "logmeteoslow") {
        commandLogMeteoSlowDelay(static_cast<uint16_t>(std::stoul(cmd.substr(12))));
        return;
    }
    if (cmd.length() > 9 && cmd.substr(0, 9) == "logocslow") {
        commandLogOcSlowDelay(static_cast<uint16_t>(std::stoul(cmd.substr(9))));
        return;
    }
    if (cmd.length() > 9 && cmd.substr(0, 9) == "logsmslow") {
        commandLogSmSlowDelay(static_cast<uint16_t>(std::stoul(cmd.substr(9))));
        return;
    }
    auto it = console_commands.find(cmd);
    if (it != console_commands.end()) {
        it->second(); // Execute the associated function
    } else {
        logMessageConsole("[CONSOLE] Command not found, use command \"help\" please");
    }
}
