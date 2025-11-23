#include "main.h"
#include "secrets.h"
#include "version.h"
#include <Adafruit_SleepyDog.h>
#include <ESPNtpClient.h>
#include <MycilaWebSerial.h>
#include <PicoMQTT.h>
#include <otawebupdater.h>
#include <wifimanager.h>

RTC_DS3231 rtc;

WIFIMANAGER WifiManager;
OTAWEBUPDATER OtaWebUpdater;

AsyncWebServer *tcp_server;

AlpacaServer alpacaServer("Alpaca_ESP32", (VERSION + String(", build ") + BUILD_NUMBER).c_str(), BUILD_DATE);

SafetyMonitor safetymonitor = SafetyMonitor();
ObservingConditions observingconditions = ObservingConditions();

Meteo meteo = Meteo();

volatile bool immediateUpdate = false;

WebSerial webSerial;

PicoMQTT::Client *mqttClient = nullptr;

String logTime() {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // gmtime_r(&now, &timeinfo);
    char strftime_buf[64]; // Ensure buffer is large enough
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(strftime_buf);
}

Preferences logPrefs;

enum LogSource {
    LogMain = 0,
    LogMeteo = 1,
    LogAlpaca = 2,
    LogObservingConsitions = 3,
    LogSafetyMonitor = 4,
    LogWifi = 5,
    LogOta = 6,
    LogConsole = 7
};

bool logEnabled[32];

void loadLogPrefs() {
    if (logPrefs.isKey("logging"))
        logPrefs.getBytes("logging", logEnabled, sizeof(logEnabled));
}

void saveLogPrefs() {
    logPrefs.putBytes("logging", logEnabled, sizeof(logEnabled));
}

String mqttLogBuffer = "";

void logLine(String line, LogSource source) {
    if (logEnabled[source] || source == LogConsole) {
        Serial.println(line);
        webSerial.println(line);
        if (mqttClient)
            mqttClient->publish(MQTT_LOG_TOPIC, mqttLogBuffer + line);
        mqttLogBuffer = "";
    }
}

void logLinePart(String line, LogSource source) {
    if (logEnabled[source] || source == LogConsole) {
        Serial.print(line);
        webSerial.print(line);
        mqttLogBuffer = mqttLogBuffer + line;
    }
}

void logLineConsole(String line) {
    logLine(line, LogConsole);
}

void logLinePartConsole(String line) {
    logLinePart(line, LogConsole);
}

void logLineMain(String line) {
    logLine(line, LogMain);
}

void logLinePartMain(String line) {
    logLinePart(line, LogMain);
}

void logLineMeteo(String line) {
    logLine(line, LogMeteo);
}

void logLinePartMeteo(String line) {
    logLinePart(line, LogMeteo);
}

void logLineAlpaca(String line) {
    logLine(line, LogAlpaca);
}

void logLinePartAlpaca(String line) {
    logLinePart(line, LogAlpaca);
}

void logLineOC(String line) {
    logLine(line, LogObservingConsitions);
}

void logLinePartOC(String line) {
    logLinePart(line, LogObservingConsitions);
}

void logLineSM(String line) {
    logLine(line, LogSafetyMonitor);
}

void logLinePartSM(String line) {
    logLinePart(line, LogSafetyMonitor);
}

void logLineWifi(String line) {
    logLine(line, LogWifi);
}

void logLinePartWifi(String line) {
    logLinePart(line, LogWifi);
}

void logLineOta(String line) {
    logLine(line, LogOta);
}

void logLinePartOta(String line) {
    logLinePart(line, LogOta);
}

void logMessage(String msg, bool showtime = true) {
    if (showtime)
        logLinePartMain(logTime() + " ");
    logLineMain(msg);
}

void logMessagePart(String msg, bool showtime = false) {
    if (showtime)
        logLinePartMain(logTime() + " ");
    logLinePartMain(msg);
}

void logMessageConsole(String msg, bool showtime = true) {
    if (showtime)
        logLinePartConsole(logTime() + " ");
    logLineConsole(msg);
}

void logMessagePartConsole(String msg, bool showtime = false) {
    if (showtime)
        logLinePartConsole(logTime() + " ");
    logLinePartConsole(msg);
}

void logMqttStatus(String special = "") {
    if (special.length() > 0) {
        logMessage(special);
        return;
    }
    String mqttServer = WifiManager.getSettings("mqtt");
    if (!mqttClient) {
        logMessage("[MQTT][STATUS] Not configured");
    } else {
        if (mqttClient->connected()) {
            logMessage("[MQTT][STATUS] Connected to: " + mqttServer);
        } else {
            logMessage("[MQTT][STATUS] Disconnected from: " + mqttServer);
        }
    }
}

void setupMqtt() {
    if (mqttClient) {
        delete mqttClient;
    }
    if (WifiManager.getSettings("mqtt").length() > 0) {
        mqttClient = new PicoMQTT::Client(WifiManager.getSettings("mqtt").c_str());
        mqttClient->connected_callback = [] {
            logMqttStatus();
        };
        mqttClient->disconnected_callback = [] {
            logMqttStatus();
        };
        mqttClient->begin();
        logMqttStatus("[MQTT][STATUS] Initiated to: " + WifiManager.getSettings("mqtt"));
    } else {
        logMqttStatus();
    }
}

void setupWebImages(AsyncWebServer *webServer) {
    // Web Images
    webServer->on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/www/favicon.ico", "image/x-icon");
    });
    webServer->on("/ascom.webp", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/www/ascom.webp", "image/webp");
    });
    webServer->on("/alpaca.webp", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/www/alpaca.webp", "image/webp");
    });
    webServer->on("/console.webp", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/www/console.webp", "image/webp");
    });
    webServer->on("/ota.webp", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/www/ota.webp", "image/webp");
    });
    webServer->on("/wifi.webp", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/www/wifi.webp", "image/webp");
    });
}

void setupWebMainPage(AsyncWebServer *webServer) {
    // Web Main Page
    webServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/www/root.html", "text/html");
    });
    webServer->on("/version", HTTP_GET, [](AsyncWebServerRequest *request) {
        String data = "©" + String(BUILD_DATE).substring(0, 4) + " DreamSky Observatory v." + String(VERSION) + " Build " + String(BUILD_NUMBER);
        String mime = "text/plain; charset=UTF-8";
        request->send(200, mime, data);
    });
    webServer->on("/uptime", HTTP_GET, [](AsyncWebServerRequest *request) {
        unsigned long uptimeMillis = millis();
        unsigned long totalSeconds = uptimeMillis / 1000;
        int days = totalSeconds / (24 * 3600);
        totalSeconds %= (24 * 3600);
        int hours = totalSeconds / 3600;
        totalSeconds %= 3600;
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        String data = "";
        char buffer[10];
        sprintf(buffer, "%d", days);
        data += buffer + String("ᵈ");
        sprintf(buffer, "%02d", hours);
        data += buffer + String("ʰ");
        sprintf(buffer, "%02d", minutes);
        data += buffer + String("ᵐ");
        sprintf(buffer, "%02d", seconds);
        data += buffer + String("ˢ");
        String mime = "text/plain; charset=UTF-8";
        request->send(200, mime, data);
    });
}

void setupWebRedirects(AsyncWebServer *webServer) {
    // Web Redirects
    webServer->on("/setup/v1/observingconditions/0/setup", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->redirect("/api/v1/observingconditions/0/setup");
    });
    webServer->on("/setup/v1/safetymonitor/0/setup", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->redirect("/api/v1/safetymonitor/0/setup");
    });
}

void IRAM_ATTR immediateMeteoUpdate() {
    immediateUpdate = true;
}

void IRAM_ATTR consoleCommand(const std::string &msg) {
    std::string cmd;
    cmd.resize(msg.size());
    std::transform(msg.begin(), msg.end(), cmd.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (cmd == "help") {
        logMessageConsole("[HELP] Available console commands:");
        logMessageConsole("[HELP] reboot - restart esp32");
    }
    if (cmd == "reboot") {
        logMessageConsole("[CONSOLE] Immediate reboot requested!");
        ESP.restart();
    }
    if (cmd == "main on") {
        logMessageConsole("[CONSOLE] Main logging enabled");
        logEnabled[LogMain] = true;
        saveLogPrefs();
    }
    if (cmd == "main off") {
        logMessageConsole("[CONSOLE] Main logging disabled");
        logEnabled[LogMain] = false;
        saveLogPrefs();
    }
    if (cmd == "meteo on") {
        logMessageConsole("[CONSOLE] Meteo logging enabled");
        logEnabled[LogMeteo] = true;
        saveLogPrefs();
    }
    if (cmd == "meteo off") {
        logMessageConsole("[CONSOLE] Meteo logging disabled");
        logEnabled[LogMeteo] = false;
        saveLogPrefs();
    }
    if (cmd == "alpaca on") {
        logMessageConsole("[CONSOLE] Alpaca logging enabled");
        logEnabled[LogAlpaca] = true;
        saveLogPrefs();
    }
    if (cmd == "alpaca off") {
        logMessageConsole("[CONSOLE] Alpaca logging disabled");
        logEnabled[LogAlpaca] = false;
        saveLogPrefs();
    }
    if (cmd == "oc on") {
        logMessageConsole("[CONSOLE] Observing conditions logging enabled");
        logEnabled[LogObservingConsitions] = true;
        saveLogPrefs();
    }
    if (cmd == "oc off") {
        logMessageConsole("[CONSOLE] Observing conditions logging disabled");
        logEnabled[LogObservingConsitions] = false;
        saveLogPrefs();
    }
    if (cmd == "sm on") {
        logMessageConsole("[CONSOLE] Safety monitor logging enabled");
        logEnabled[LogSafetyMonitor] = true;
        saveLogPrefs();
    }
    if (cmd == "sm off") {
        logMessageConsole("[CONSOLE] Safety monitor logging disabled");
        logEnabled[LogSafetyMonitor] = false;
        saveLogPrefs();
    }
    if (cmd == "wifi on") {
        logMessageConsole("[CONSOLE] WiFi logging enabled");
        logEnabled[LogWifi] = true;
        saveLogPrefs();
    }
    if (cmd == "wifi off") {
        logMessageConsole("[CONSOLE] WiFi logging disabled");
        logEnabled[LogWifi] = false;
        saveLogPrefs();
    }
    if (cmd == "ota on") {
        logMessageConsole("[CONSOLE] OTA logging enabled");
        logEnabled[LogOta] = true;
        saveLogPrefs();
    }
    if (cmd == "ota off") {
        logMessageConsole("[CONSOLE] OTA logging disabled");
        logEnabled[LogOta] = false;
        saveLogPrefs();
    }
    if (cmd == "log on") {
        logMessageConsole("[CONSOLE] All logging enabled");
        std::fill(std::begin(logEnabled), std::end(logEnabled), true);
        saveLogPrefs();
    }
    if (cmd == "log off") {
        logMessageConsole("[CONSOLE] All logging disabled");
        std::fill(std::begin(logEnabled), std::end(logEnabled), false);
        saveLogPrefs();
    }
}

void setup() {
    // Setup serial
    Serial.begin(115200);
    while (!Serial) {
        delay(50);
    }
    Serial.println("");
    // Logging preferences
    logPrefs.begin("logPrefs", false);
    std::fill(std::begin(logEnabled), std::end(logEnabled), true);
    loadLogPrefs();
    // System Timezone
    setenv("TZ", RTC_TIMEZONE, 1);
    tzset();
    // RTC
    if (!rtc.begin())
        logMessage("[TIME][RTC] Couldn't find RTC", false);
    if (rtc.lostPower()) {
        logMessage("[TIME][RTC] RTC lost power, let's set the time!", false);
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    DateTime rtcNow = rtc.now();
    struct timeval tv;
    tv.tv_sec = rtcNow.unixtime();
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);
    // TODO Fixed WiFi settings with reconnect
    // setup_wifi();
    // NTP
    NTP.onNTPSyncEvent([](NTPEvent_t event) {
        switch (event.event) {
        case timeSyncd: {
            struct timeval now;
            gettimeofday(&now, NULL);
            time_t t = now.tv_sec;
            rtc.adjust(DateTime(t));
            logMessage("[TIME][RTC] Synced");
            logMessage("[TIME][NTP] " + String(NTP.ntpEvent2str(event)));
        } break;
        case partlySync:
        case syncNotNeeded:
        case accuracyError:
            logMessage("[TIME][NTP] " + String(NTP.ntpEvent2str(event)));
            break;
        default:
            break;
        }
    });
    NTP.setTimeZone(NTP_TIMEZONE);
    NTP.setInterval(NTP_INTERVAL_SHORT, NTP_INTERVAL_LONG);
    NTP.setNTPTimeout(NTP_TIMEOUT);
    // NTP.setMinSyncAccuracy (5000);
    // NTP.settimeSyncThreshold (3000);
    NTP.begin(NTP_SERVER);
    // TCP server
    tcp_server = new AsyncWebServer(ALPACA_TCP_PORT);
    // Web Serial
    webSerial.onMessage(consoleCommand);
    webSerial.setBuffer(100);
    webSerial.begin(tcp_server);
    // WiFi Manager
    WifiManager.setLogger(logLineWifi, logLinePartWifi, logTime); // Set message logger
    WifiManager.startBackgroundTask();                            // Run the background task to take care of our Wifi
    WifiManager.fallbackToSoftAp(true);                           // Run a SoftAP if no known AP can be reached
    WifiManager.attachWebServer(tcp_server);                      // Attach our API to the HTTP Webserver
    WifiManager.attachUI();
    // OTA Manager
    // OtaWebUpdater.setBaseUrl(OTA_BASE_URL);    // Set the OTA Base URL for automatic updates
    OtaWebUpdater.setLogger(logLineOta, logLinePartOta, logTime);                               // Set message logger
    OtaWebUpdater.setFirmware(BUILD_DATE, String(VERSION) + ", build " + String(BUILD_NUMBER)); // Set the current firmware version
    OtaWebUpdater.startBackgroundTask();                                                        // Run the background task to check for updates
    OtaWebUpdater.attachWebServer(tcp_server);                                                  // Attach our API to the Webserver
    OtaWebUpdater.attachUI();                                                                   // Attach the UI to the Webserver
    setupWebImages(tcp_server);
    setupWebMainPage(tcp_server);
    setupWebRedirects(tcp_server);
    tcp_server->begin();
    // ALPACA Tcp Server
    alpacaServer.setLogger(logLineAlpaca, logLinePartAlpaca, logTime);
    alpacaServer.beginTcp(tcp_server, ALPACA_TCP_PORT);
    // Observing Conditions
    observingconditions.setImmediateUpdate(immediateMeteoUpdate);
    alpacaServer.addDevice(&observingconditions);
    // Safety Monitor
    safetymonitor.setLogger(logLineSM, logLinePartSM, logTime);
    alpacaServer.addDevice(&safetymonitor);
    alpacaServer.loadSettings();
    // Meteo sensors
    meteo.setLogger(logLineMeteo, logLinePartMeteo, logTime);
    meteo.begin();
    attachInterrupt(digitalPinToInterrupt(RAIN_SENSOR_PIN), immediateMeteoUpdate, CHANGE);
    // Watchdog
    int watchdogCountdown = Watchdog.enable(WATCHDOG_COUNTDOWN);
    logMessage("[WATCHDOG] Enabled with " + String(watchdogCountdown) + "ms countdown.");
}

void loop() {
    // weather update last ran millis
    static unsigned long meteoLastRan = 0;
    static unsigned long safetyMonitorLastRan = 0;
    static unsigned long observingConditionsLastRan = 0;
    // mqtt status loop delay
    static int prevWifiStatus = WL_DISCONNECTED;
    static int mqttStatusDelay = MQTT_STATUS_DELAY;
    static int lastMqttStatus = 0;
    // do not continue regular operation as long as a OTA is running
    // reason: background workload can cause upgrade issues that we want to avoid!
    if (OtaWebUpdater.otaIsRunning) {
        yield();
        delay(50);
        return;
    };
    // wifi (re)connected
    if (WiFi.status() == WL_CONNECTED && prevWifiStatus != WL_CONNECTED) {
        // ALPACA Tcp Server
        alpacaServer.beginUdp(ALPACA_UDP_PORT);
        // initialize mqtt
        setupMqtt();
    }
    prevWifiStatus = WiFi.status();
    if (mqttClient)
        mqttClient->loop();
    // log mqtt status without blocking webserver
    if (millis() > lastMqttStatus + mqttStatusDelay) {
        logMqttStatus();
        lastMqttStatus = millis();
    }
    // update meteo every METEO_MEASURE_DELAY without blocking webserver
    if (immediateUpdate || (millis() > meteoLastRan + METEO_MEASURE_DELAY)) {
        meteo.update();
        meteoLastRan = millis();
    }
    // update safetymonitor every METEO_MEASURE_DELAY without blocking webserver
    if (immediateUpdate || (millis() > safetyMonitorLastRan + SAFETY_MONITOR_DELAY)) {
        safetymonitor.update(meteo);
        safetyMonitorLastRan = millis();
    }
    // update observingconditions every refresh without blocking webserver
    if (immediateUpdate || (millis() > observingConditionsLastRan + (1000 * observingconditions.getRefresh()))) {
        observingconditions.update(meteo);
        observingConditionsLastRan = millis();
    }
    immediateUpdate = false;
    Watchdog.reset();
    delay(50);
}

void setup_wifi() {
    // pinMode(PIN_WIFI_LED, OUTPUT);
    logMessagePart(F("[MAIN] Setup WiFi "), true);
    WiFi.begin(WIFISSID, WIFIPASS);
    int pause = 10000;
    unsigned long begin = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - begin < pause) {
        delay(500);
        logMessagePart(".");
    }
    logMessage(".");
    logMessage("[MAIN] WiFi connected: " + String(WiFi.localIP()));
}
