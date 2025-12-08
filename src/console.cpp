#include "console.h"
#include "hardware.h"
#include "log.h"
#include <Arduino.h>
#include <iterator>
#include <map>
#include <string>

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
    logMessageConsole("[HELP]   log on/off                    - enable/disable all logging");
    logMessageConsole("[HELP]   log main on/off               - enable/disable main logging");
    logMessageConsole("[HELP]   log alpaca on/off             - enable/disable alpaca server logging");
    logMessageConsole("[HELP]   log meteo on/slow [nnn]/off   - enable/slow, nnn seconds/disable meteo logging");
    logMessageConsole("[HELP]   log obscon on/slow [nnn]/off  - enable/slow, nnn seconds/disable observing conditions logging");
    logMessageConsole("[HELP]   log safemon on/slow [nnn]/off - enable/slow, nnn seconds/disable safety monitor logging");
    logMessageConsole("[HELP]   log wifi on/off               - enable/disable wifi manager logging");
    logMessageConsole("[HELP]   log ota on/off                - enable/disable ota update logging");
    logMessageConsole("[HELP] Hardware settings (reboot required):");
    logMessageConsole("[HELP]   hw bmp280 on/off    - enable/disable BMP280 sensor");
    logMessageConsole("[HELP]   hw aht20 on/off     - enable/disable AHT20 sensor");
    logMessageConsole("[HELP]   hw mlx90614 on/off  - enable/disable MLX90614 sensor");
    logMessageConsole("[HELP]   hw tsl2591 on/off   - enable/disable TSL2591 sensor");
    logMessageConsole("[HELP]   hw anemo4403 on/off - enable/disable ANEMO4403 sensor");
    logMessageConsole("[HELP]   hw uicpal on/off    - enable/disable UICPAL sensor");
    logMessageConsole("[HELP]   hw rg15 on/off      - enable/disable RG-15 sensor");
    logMessageConsole("[HELP] Alpaca settings (reboot required):");
    logMessageConsole("[HELP]   alpaca obscon on/off  - enable/disable observing conditions service");
    logMessageConsole("[HELP]   alpaca safemon on/off - enable/disable safety monitor service");
}

void commandLogState() {
    logMessageConsole("[INFO] Logging state");
    logMessageConsole("[INFO] -------------");
    logMessageConsole("[INFO]  console - on");
    logMessageConsole("[INFO]  main    - " + String(logEnabled[LogMain] ? "on" : "off"));
    logMessageConsole("[INFO]  alpaca  - " + String(logEnabled[LogAlpaca] ? "on" : "off"));
    logMessageConsole("[INFO]  meteo   - " + String(logEnabled[LogMeteo] ? (logEnabled[LogMeteo] == LogOn ? "on" : ("slow " + String(logSlow[LogMeteo]))) : "off") + " [meteo sensors]");
    logMessageConsole("[INFO]  obscon  - " + String(logEnabled[LogObservingConditions] ? (logEnabled[LogObservingConditions] == LogOn ? "ob" : ("slow " + String(logSlow[LogObservingConditions]))) : "off") + " [observing conditions]");
    logMessageConsole("[INFO]  safemon - " + String(logEnabled[LogSafetyMonitor] ? (logEnabled[LogSafetyMonitor] == LogOn ? "on" : ("slow " + String(logSlow[LogSafetyMonitor]))) : "off") + " [safety monitor]");
    logMessageConsole("[INFO]  wifi    - " + String(logEnabled[LogWifi] ? "on" : "off") + " [wifi manager]");
    logMessageConsole("[INFO]  ota     - " + String(logEnabled[LogOta] ? "on" : "off") + " [update manager]");
}

void commandHardwareState() {
    logMessageConsole("[INFO] Firmware supported hardware");
    logMessageConsole("[INFO] ---------------------------");
    logMessageConsole("[INFO] Sensors:");
    logMessageConsole("[INFO]   DS3231    - " + String(HARDWARE_DS3231 ? "enabled " : "disabled") + " (realtime clock)");
    logMessageConsole("[INFO]   BMP280    - " + String(HARDWARE_BMP280 ? "enabled " : "disabled") + " (temperature and pressure)");
    logMessageConsole("[INFO]   AHT20     - " + String(HARDWARE_AHT20 ? "enabled " : "disabled") + " (temperature and humidity)");
    logMessageConsole("[INFO]   MLX90614  - " + String(HARDWARE_MLX90614 ? "enabled " : "disabled") + " (sky temperature)");
    logMessageConsole("[INFO]   TSL2591   - " + String(HARDWARE_TSL2591 ? "enabled " : "disabled") + " (sky brightness)");
    logMessageConsole("[INFO]   ANEMO4403 - " + String(HARDWARE_ANEMO4403 ? "enabled " : "disabled") + " (wind speed)");
    logMessageConsole("[INFO]   UICPAL    - " + String(HARDWARE_UICPAL ? "enabled " : "disabled") + " (rain/snow sensor)");
    logMessageConsole("[INFO]   RG15      - " + String(HARDWARE_RG15 ? "enabled " : "disabled") + " (rain rate sensor)");
    logMessageConsole("[INFO] Alpaca:");
    logMessageConsole("[INFO]   Observing conditions - " + String(ALPACA_OBSCON ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Safety monitor       - " + String(ALPACA_SAFEMON ? "enabled" : "disabled"));
    logMessageConsole("[INFO] Obsering conditions:");
    logMessageConsole("[INFO]   Rain rate       - " + String(OBSCON_RAINRATE ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Temperature     - " + String(OBSCON_TEMPERATURE ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Humidity        - " + String(OBSCON_HUMIDITY ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Dew point       - " + String(OBSCON_DEWPOINT ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Pressure        - " + String(OBSCON_PRESSURE ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Sky temperature - " + String(OBSCON_SKYTEMP ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Cloud cover     - " + String(OBSCON_CLOUDCOVER ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Star FWHM       - " + String(OBSCON_FWHM ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Sky brightness  - " + String(OBSCON_SKYBRIGHTNESS ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Sky quality     - " + String(OBSCON_SKYQUALITY ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Wind direction  - " + String(OBSCON_WINDDIR ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Wind speed      - " + String(OBSCON_WINDSPEED ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Wind gust       - " + String(OBSCON_WINDGUST ? "enabled" : "disabled"));
    logMessageConsole("[INFO] Safety monitor:");
    logMessageConsole("[INFO]   Rain rate       - " + String(SAFEMON_RAINRATE ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Temperature     - " + String(SAFEMON_TEMPERATURE ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Humidity        - " + String(SAFEMON_HUMIDITY ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Dew point       - " + String(SAFEMON_DEWPOINT ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Sky temperature - " + String(SAFEMON_SKYTEMP ? "enabled" : "disabled"));
    logMessageConsole("[INFO]   Wind speed      - " + String(SAFEMON_WINDSPEED ? "enabled" : "disabled"));
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

void commandLogObsconOn() {
    logMessageConsole("[CONSOLE] Observing conditions logging enabled");
    logEnabled[LogObservingConditions] = LogOn;
    saveLogPrefs();
}

void commandLogObsconOff() {
    logMessageConsole("[CONSOLE] Observing conditions logging disabled");
    logEnabled[LogObservingConditions] = LogOff;
    saveLogPrefs();
}

void commandLogObsconSlow() {
    logMessageConsole("[CONSOLE] Observing conditions logging slow " + String(logSlow[LogObservingConditions]));
    logEnabled[LogObservingConditions] = LogSlow;
    saveLogPrefs();
}

void commandLogObsconSlowDelay(uint16_t delay) {
    logMessageConsole("[CONSOLE] Observing conditions logging slow " + String(delay));
    logEnabled[LogObservingConditions] = LogSlow;
    logSlow[LogObservingConditions] = delay;
    saveLogPrefs();
}

void commandLogSafemonOn() {
    logMessageConsole("[CONSOLE] Safety monitor logging enabled");
    logEnabled[LogSafetyMonitor] = LogOn;
    saveLogPrefs();
}

void commandLogSafemonOff() {
    logMessageConsole("[CONSOLE] Safety monitor logging disabled");
    logEnabled[LogSafetyMonitor] = LogOff;
    saveLogPrefs();
}

void commandLogSafemonSlow() {
    logMessageConsole("[CONSOLE] Safety monitor logging slow " + String(logSlow[LogSafetyMonitor]));
    logEnabled[LogSafetyMonitor] = LogSlow;
    saveLogPrefs();
}

void commandLogSafemonSlowDelay(uint16_t delay) {
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

void commandAlpacaObsconOn() {
    logMessageConsole("[CONSOLE] Alpaca observing conditions enabled");
    ALPACA_OBSCON = true;
    saveHwPrefs();
}

void commandAlpacaObsconOff() {
    logMessageConsole("[CONSOLE] Alpaca observing conditions disabled");
    ALPACA_OBSCON = false;
    saveHwPrefs();
}

void commandAlpacaSafemonOn() {
    logMessageConsole("[CONSOLE] Alpaca safety monitor enabled");
    ALPACA_SAFEMON = true;
    saveHwPrefs();
}

void commandAlpacaSafemonOff() {
    logMessageConsole("[CONSOLE] Alpaca safety monitor disabled");
    ALPACA_SAFEMON = false;
    saveHwPrefs();
}

void commandHwDs3231On() {
    logMessageConsole("[CONSOLE] DS3231 enabled");
    HARDWARE_DS3231 = true;
    saveHwPrefs();
}

void commandHwDs3231Off() {
    logMessageConsole("[CONSOLE] DS3231 disabled");
    HARDWARE_DS3231 = false;
    saveHwPrefs();
}

void commandHwBmp280On() {
    logMessageConsole("[CONSOLE] BMP280 enabled");
    HARDWARE_BMP280 = true;
    saveHwPrefs();
}

void commandHwBmp280Off() {
    logMessageConsole("[CONSOLE] BMP280 disabled");
    HARDWARE_BMP280 = false;
    saveHwPrefs();
}

void commandHwAht20On() {
    logMessageConsole("[CONSOLE] AHT20 enabled");
    HARDWARE_AHT20 = true;
    saveHwPrefs();
}

void commandHwAht20Off() {
    logMessageConsole("[CONSOLE] AHT20 disabled");
    HARDWARE_AHT20 = false;
    saveHwPrefs();
}

void commandHwMlx90614On() {
    logMessageConsole("[CONSOLE] MLX90614 enabled");
    HARDWARE_MLX90614 = true;
    saveHwPrefs();
}

void commandHwMlx90614Off() {
    logMessageConsole("[CONSOLE] MLX90614 disabled");
    HARDWARE_MLX90614 = false;
    saveHwPrefs();
}

void commandHwTsl2591On() {
    logMessageConsole("[CONSOLE] TSL2591 enabled");
    HARDWARE_TSL2591 = true;
    saveHwPrefs();
}

void commandHwTsl2591Off() {
    logMessageConsole("[CONSOLE] TSL2591 disabled");
    HARDWARE_TSL2591 = false;
    saveHwPrefs();
}

void commandHwAnemo4403On() {
    logMessageConsole("[CONSOLE] ANEMO4403 enabled");
    HARDWARE_ANEMO4403 = true;
    saveHwPrefs();
}

void commandHwAnemo4403Off() {
    logMessageConsole("[CONSOLE] ANEMO4403 disabled");
    HARDWARE_ANEMO4403 = false;
    saveHwPrefs();
}

void commandHwUicpalOn() {
    logMessageConsole("[CONSOLE] UICPAL enabled");
    HARDWARE_UICPAL = true;
    saveHwPrefs();
}

void commandHwUicpalOff() {
    logMessageConsole("[CONSOLE] UICPAL disabled");
    HARDWARE_UICPAL = false;
    saveHwPrefs();
}

void commandHwRg15On() {
    logMessageConsole("[CONSOLE] RG15 enabled");
    HARDWARE_RG15 = true;
    saveHwPrefs();
}

void commandHwRg15Off() {
    logMessageConsole("[CONSOLE] RG15 disabled");
    HARDWARE_RG15 = false;
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
}

void processConsoleCommand(const std::string &msg) {
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
    auto it = console_commands.find(cmd);
    if (it != console_commands.end()) {
        it->second(); // Execute the associated function
    } else {
        logMessageConsole("[CONSOLE] Command not found, use command \"help\" please");
    }
}
