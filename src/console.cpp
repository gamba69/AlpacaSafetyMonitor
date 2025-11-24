#include "console.h"
#include "log.h"
#include <Arduino.h>
#include <iterator>
#include <map>

std::map<std::string, std::function<void()>> console_commands;

void commandHelp() {
    logMessageConsole("[HELP] Available console commands");
    logMessageConsole("[HELP] --------------------------");
    logMessageConsole("[HELP] reboot            - restart esp32 ascom alpaca device");
    logMessageConsole("[HELP] log on/off        - enable/disable all logging");
    logMessageConsole("[HELP] log main on/off   - enable/disable main logging");
    logMessageConsole("[HELP] log alpaca on/off - enable/disable alpaca server logging");
    logMessageConsole("[HELP] log meteo on/off  - enable/disable meteo logging");
    logMessageConsole("[HELP] log oc on/off     - enable/disable observing conditions logging");
    logMessageConsole("[HELP] log sm on/off     - enable/disable safety monitor logging");
    logMessageConsole("[HELP] log wifi on/off   - enable/disable wifi manager logging");
    logMessageConsole("[HELP] log ota on/off    - enable/disable ota update logging");
    logMessageConsole("[HELP] log               - show curent logging state");
}

void commandLogState() {
    logMessageConsole("[HELP] Logging state");
    logMessageConsole("[HELP] -------------");
    logMessageConsole("[HELP] console - enabled");
    logMessageConsole("[HELP] main    - " + String(logEnabled[LogMain] ? "enabled" : "disabled"));
    logMessageConsole("[HELP] alpaca  - " + String(logEnabled[LogAlpaca] ? "enabled" : "disabled"));
    logMessageConsole("[HELP] meteo   - " + String(logEnabled[LogMeteo] ? "enabled" : "disabled"));
    logMessageConsole("[HELP] oc      - " + String(logEnabled[LogObservingConditions] ? "enabled" : "disabled") + " (observing conditions)");
    logMessageConsole("[HELP] sm      - " + String(logEnabled[LogSafetyMonitor] ? "enabled" : "disabled") + " (safety monitor)");
    logMessageConsole("[HELP] wifi    - " + String(logEnabled[LogWifi] ? "enabled" : "disabled") + " (wifi manager)");
    logMessageConsole("[HELP] ota     - " + String(logEnabled[LogOta] ? "enabled" : "disabled"));
}

void commandReboot() {
    logMessageConsole("[CONSOLE] Immediate reboot requested!");
    logMessageConsole("[REBOOT]");
    ESP.restart();
}

void commandLogMainOn() {
    logMessageConsole("[CONSOLE] Main logging enabled");
    logEnabled[LogMain] = true;
    saveLogPrefs();
}

void commandLogMainOff() {
    logMessageConsole("[CONSOLE] Main logging disabled");
    logEnabled[LogMain] = false;
    saveLogPrefs();
}

void commandLogMeteoOn() {
    logMessageConsole("[CONSOLE] Meteo logging enabled");
    logEnabled[LogMeteo] = true;
    saveLogPrefs();
}

void commandLogMeteoOff() {
    logMessageConsole("[CONSOLE] Meteo logging disabled");
    logEnabled[LogMeteo] = false;
    saveLogPrefs();
}

void commandLogAlpacaOn() {
    logMessageConsole("[CONSOLE] Alpaca logging enabled");
    logEnabled[LogAlpaca] = true;
    saveLogPrefs();
}

void commandLogAlpacaOff() {
    logMessageConsole("[CONSOLE] Alpaca logging disabled");
    logEnabled[LogAlpaca] = false;
    saveLogPrefs();
}

void commandLogOcOn() {
    logMessageConsole("[CONSOLE] Observing conditions logging enabled");
    logEnabled[LogObservingConditions] = true;
    saveLogPrefs();
}

void commandLogOcOff() {
    logMessageConsole("[CONSOLE] Observing conditions logging disabled");
    logEnabled[LogObservingConditions] = false;
    saveLogPrefs();
}

void commandLogSmOn() {
    logMessageConsole("[CONSOLE] Safety monitor logging enabled");
    logEnabled[LogSafetyMonitor] = true;
    saveLogPrefs();
}

void commandLogSmOff() {
    logMessageConsole("[CONSOLE] Safety monitor logging disabled");
    logEnabled[LogSafetyMonitor] = false;
    saveLogPrefs();
}

void commandLogWifiOn() {
    logMessageConsole("[CONSOLE] Wifi monitor logging enabled");
    logEnabled[LogWifi] = true;
    saveLogPrefs();
}

void commandLogWifiOff() {
    logMessageConsole("[CONSOLE] Wifi logging disabled");
    logEnabled[LogWifi] = false;
    saveLogPrefs();
}

void commandLogOtaOn() {
    logMessageConsole("[CONSOLE] Ota monitor logging enabled");
    logEnabled[LogOta] = true;
    saveLogPrefs();
}

void commandLogOtaOff() {
    logMessageConsole("[CONSOLE] Ota logging disabled");
    logEnabled[LogOta] = false;
    saveLogPrefs();
}

void commandLogOn() {
    logMessageConsole("[CONSOLE] All logging enabled");
    std::fill(std::begin(logEnabled), std::end(logEnabled), true);
    saveLogPrefs();
}

void commandLogOff() {
    logMessageConsole("[CONSOLE] All logging disabled");
    std::fill(std::begin(logEnabled), std::end(logEnabled), false);
    saveLogPrefs();
}

void initConsoleCommands() {

    console_commands["help"] = commandHelp;
    console_commands["reboot"] = commandReboot;

    console_commands["logmainon"] = commandLogMainOn;
    console_commands["mainon"] = commandLogMainOn;
    console_commands["logmainoff"] = commandLogMainOff;
    console_commands["mainoff"] = commandLogMainOff;

    console_commands["logmeteoon"] = commandLogMeteoOn;
    console_commands["meteoon"] = commandLogMeteoOn;
    console_commands["logmeteooff"] = commandLogMeteoOff;
    console_commands["meteooff"] = commandLogMeteoOff;

    console_commands["logalpacaon"] = commandLogAlpacaOn;
    console_commands["alpacaon"] = commandLogAlpacaOn;
    console_commands["logalpacaoff"] = commandLogAlpacaOff;
    console_commands["alpacaoff"] = commandLogAlpacaOff;

    console_commands["logocon"] = commandLogOcOn;
    console_commands["ocon"] = commandLogOcOn;
    console_commands["logocoff"] = commandLogOcOff;
    console_commands["ocoff"] = commandLogOcOff;

    console_commands["logsmon"] = commandLogSmOn;
    console_commands["smon"] = commandLogSmOn;
    console_commands["logsmoff"] = commandLogSmOff;
    console_commands["smoff"] = commandLogSmOff;

    console_commands["logwifion"] = commandLogWifiOn;
    console_commands["wifion"] = commandLogWifiOn;
    console_commands["logwifioff"] = commandLogWifiOff;
    console_commands["wifioff"] = commandLogWifiOff;

    console_commands["logotaon"] = commandLogOtaOn;
    console_commands["otaon"] = commandLogOtaOn;
    console_commands["logotaoff"] = commandLogOtaOff;
    console_commands["otaoff"] = commandLogOtaOff;

    console_commands["logon"] = commandLogOn;
    console_commands["logoff"] = commandLogOff;

    console_commands["log"] = commandLogState;
    console_commands["logs"] = commandLogState;
}

void IRAM_ATTR processConsoleCommand(const std::string &msg) {
    std::string cmd;
    cmd.resize(msg.size());
    std::transform(msg.begin(), msg.end(), cmd.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    cmd.erase(remove_if(cmd.begin(), cmd.end(), isspace), cmd.end());
    auto it = console_commands.find(cmd);
    if (it != console_commands.end()) {
        it->second(); // Execute the associated function
    } else {
        logMessageConsole("[CONSOLE] Command not found, use command \"help\" please");
    }
}
