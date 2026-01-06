#include "console.h"
#include "hardware.h"
#include "log.h"
#include <Arduino.h>
#include <algorithm>
#include <iterator>
#include <map>
#include <sstream>
#include <string>

std::map<std::string, std::function<void()>> console_commands;

void commandHelp() {
    logMessageConsole("[HELP] Available console commands");
    logMessageConsole("[HELP] --------------------------");
    logMessageConsole("[HELP] Info:");
    logMessageConsole("[HELP]   help   - this screen");
    logMessageConsole("[HELP]   log    - show curent log settings");
    logMessageConsole("[HELP]   target - show curent log target settings");
    logMessageConsole("[HELP]   hw     - show curent log settings");
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
    logMessageConsole("[HELP] Log target settings:");
    logMessageConsole("[HELP]   target serial on/off  - enable/disable serial log output");
    logMessageConsole("[HELP]   target console on/off - enable/disable console log output");
    logMessageConsole("[HELP]   target mqtt on/off    - enable/disable mqtt log output");
    logMessageConsole("[HELP]   target led on/off     - enable/disable led log output");
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
    logMessageConsole("[INFO]  main    - " + String(logEnabled[LogSource::Main] ? "on" : "off"));
    logMessageConsole("[INFO]  alpaca  - " + String(logEnabled[LogSource::Alpaca] ? "on" : "off"));
    logMessageConsole("[INFO]  meteo   - " + String(logEnabled[LogSource::Meteo] ? (logEnabled[LogSource::Meteo] == Log::On ? "on" : ("slow " + String(logSlow[LogSource::Meteo]))) : "off") + " [meteo sensors]");
    logMessageConsole("[INFO]  obscon  - " + String(logEnabled[LogSource::ObsCon] ? (logEnabled[LogSource::ObsCon] == Log::On ? "ob" : ("slow " + String(logSlow[LogSource::ObsCon]))) : "off") + " [observing conditions]");
    logMessageConsole("[INFO]  safemon - " + String(logEnabled[LogSource::SafeMon] ? (logEnabled[LogSource::SafeMon] == Log::On ? "on" : ("slow " + String(logSlow[LogSource::SafeMon]))) : "off") + " [safety monitor]");
    logMessageConsole("[INFO]  wifi    - " + String(logEnabled[LogSource::Wifi] ? "on" : "off") + " [wifi manager]");
    logMessageConsole("[INFO]  ota     - " + String(logEnabled[LogSource::Ota] ? "on" : "off") + " [update manager]");
}

void commandTargetState() {
    logMessageConsole("[INFO] Logging target state");
    logMessageConsole("[INFO] --------------------");
    logMessageConsole("[INFO]  serial  - " + String(LOG_SERIAL ? "on" : "off"));
    logMessageConsole("[INFO]  console - " + String(LOG_CONSOLE ? "on" : "off"));
    logMessageConsole("[INFO]  mqtt    - " + String(LOG_MQTT ? "on" : "off"));
    logMessageConsole("[INFO]  led     - " + String(LOG_LED ? "on" : "off"));
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

void commandTargetSerialOn() {
    logMessageConsole("[CONSOLE] Serial target logging enabled");
    LOG_SERIAL = Log::On;
    saveLogPrefs();
}

void commandTargetSerialOff() {
    logMessageConsole("[CONSOLE] Serial target logging disabled");
    LOG_SERIAL = Log::Off;
    saveLogPrefs();
}

void commandTargetConsoleOn() {
    logMessageConsole("[CONSOLE] Console target logging enabled");
    LOG_CONSOLE = Log::On;
    saveLogPrefs();
}

void commandTargetConsoleOff() {
    logMessageConsole("[CONSOLE] Console target logging disabled");
    LOG_CONSOLE = Log::Off;
    saveLogPrefs();
}

void commandTargetMqttOn() {
    logMessageConsole("[CONSOLE] MQTT target logging enabled");
    LOG_MQTT = Log::On;
    saveLogPrefs();
}

void commandTargetMqttOff() {
    logMessageConsole("[CONSOLE] MQTT target logging disabled");
    LOG_MQTT = Log::Off;
    saveLogPrefs();
}

void commandTargetLedOn() {
    logMessageConsole("[CONSOLE] Led target logging enabled");
    LOG_LED = Log::On;
    saveLogPrefs();
}

void commandTargetLedOff() {
    logMessageConsole("[CONSOLE] Led target logging disabled");
    LOG_LED = Log::Off;
    saveLogPrefs();
}

void commandLogMainOn() {
    logMessageConsole("[CONSOLE] Main logging enabled");
    logEnabled[LogSource::Main] = Log::On;
    saveLogPrefs();
}

void commandLogMainOff() {
    logMessageConsole("[CONSOLE] Main logging disabled");
    logEnabled[LogSource::Main] = Log::Off;
    saveLogPrefs();
}

void commandLogMeteoOn() {
    logMessageConsole("[CONSOLE] Meteo logging enabled");
    logEnabled[LogSource::Meteo] = Log::On;
    saveLogPrefs();
}

void commandLogMeteoOff() {
    logMessageConsole("[CONSOLE] Meteo logging disabled");
    logEnabled[LogSource::Meteo] = Log::Off;
    saveLogPrefs();
}

void commandLogMeteoSlow() {
    logMessageConsole("[CONSOLE] Meteo logging slow " + logSlow[LogSource::Meteo]);
    logEnabled[LogSource::Meteo] = Log::Slow;
    saveLogPrefs();
}

void commandLogMeteoSlowDelay(uint16_t delay) {
    logMessageConsole("[CONSOLE] Meteo logging slow " + String(delay));
    logEnabled[LogSource::Meteo] = Log::Slow;
    logSlow[LogSource::Meteo] = delay;
    saveLogPrefs();
}

void commandLogAlpacaOn() {
    logMessageConsole("[CONSOLE] Alpaca logging enabled");
    logEnabled[LogSource::Alpaca] = Log::On;
    saveLogPrefs();
}

void commandLogAlpacaOff() {
    logMessageConsole("[CONSOLE] Alpaca logging disabled");
    logEnabled[LogSource::Alpaca] = Log::Off;
    saveLogPrefs();
}

void commandLogObsconOn() {
    logMessageConsole("[CONSOLE] Observing conditions logging enabled");
    logEnabled[LogSource::ObsCon] = Log::On;
    saveLogPrefs();
}

void commandLogObsconOff() {
    logMessageConsole("[CONSOLE] Observing conditions logging disabled");
    logEnabled[LogSource::ObsCon] = Log::Off;
    saveLogPrefs();
}

void commandLogObsconSlow() {
    logMessageConsole("[CONSOLE] Observing conditions logging slow " + String(logSlow[LogSource::ObsCon]));
    logEnabled[LogSource::ObsCon] = Log::Slow;
    saveLogPrefs();
}

void commandLogObsconSlowDelay(uint16_t delay) {
    logMessageConsole("[CONSOLE] Observing conditions logging slow " + String(delay));
    logEnabled[LogSource::ObsCon] = Log::Slow;
    logSlow[LogSource::ObsCon] = delay;
    saveLogPrefs();
}

void commandLogSafemonOn() {
    logMessageConsole("[CONSOLE] Safety monitor logging enabled");
    logEnabled[LogSource::SafeMon] = Log::On;
    saveLogPrefs();
}

void commandLogSafemonOff() {
    logMessageConsole("[CONSOLE] Safety monitor logging disabled");
    logEnabled[LogSource::SafeMon] = Log::Off;
    saveLogPrefs();
}

void commandLogSafemonSlow() {
    logMessageConsole("[CONSOLE] Safety monitor logging slow " + String(logSlow[LogSource::SafeMon]));
    logEnabled[LogSource::SafeMon] = Log::Slow;
    saveLogPrefs();
}

void commandLogSafemonSlowDelay(uint16_t delay) {
    logMessageConsole("[CONSOLE] Safety monitor logging slow " + String(delay));
    logEnabled[LogSource::SafeMon] = Log::Slow;
    logSlow[LogSource::SafeMon] = delay;
    saveLogPrefs();
}

void commandLogWifiOn() {
    logMessageConsole("[CONSOLE] Wifi monitor logging enabled");
    logEnabled[LogSource::Wifi] = Log::On;
    saveLogPrefs();
}

void commandLogWifiOff() {
    logMessageConsole("[CONSOLE] Wifi logging disabled");
    logEnabled[LogSource::Wifi] = Log::Off;
    saveLogPrefs();
}

void commandLogOtaOn() {
    logMessageConsole("[CONSOLE] Ota monitor logging enabled");
    logEnabled[LogSource::Ota] = Log::On;
    saveLogPrefs();
}

void commandLogOtaOff() {
    logMessageConsole("[CONSOLE] Ota logging disabled");
    logEnabled[LogSource::Ota] = Log::Off;
    saveLogPrefs();
}

void commandLogOn() {
    logMessageConsole("[CONSOLE] All logging enabled");
    std::fill(std::begin(logEnabled), std::end(logEnabled), Log::On);
    saveLogPrefs();
}

void commandLogOff() {
    logMessageConsole("[CONSOLE] All logging disabled");
    std::fill(std::begin(logEnabled), std::end(logEnabled), Log::Off);
    saveLogPrefs();
}

void commandLogSlow() {
    logMessageConsole("[CONSOLE] All logging enabled, slow");
    std::fill(std::begin(logEnabled), std::end(logEnabled), Log::On);
    logEnabled[LogSource::Meteo] = Log::Slow;
    logEnabled[LogSource::ObsCon] = Log::Slow;
    logEnabled[LogSource::SafeMon] = Log::Slow;
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

    console_commands["target"] = commandTargetState;
    console_commands["targets"] = commandTargetState;

    console_commands["log"] = commandLogState;
    console_commands["logs"] = commandLogState;

    console_commands["hw"] = commandHardwareState;
    console_commands["hardware"] = commandHardwareState;

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
    std::string cmd = (w1.find("temp") == 0 ? "temp" : w1.find("humi") == 0 ? "humi"
                                                                            : "");
    if (cmd.empty()) {
        return result;
    }
    if (w2 == "weight") {
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
    int expected = (cmd == "tempweight" ? 3 : cmd == "humiweight" ? 2
                                                                  : 1);
    if (result.valueCount != expected) {
        return result;
    }
    result.command = cmd;
    result.success = true;
    return result;
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
