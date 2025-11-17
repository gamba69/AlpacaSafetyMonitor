#include "main.h"
#include "secrets.h"
#include "version.h"
#include <ESPNtpClient.h>
#include <MycilaWebSerial.h>
#include <PicoMQTT.h>
#include <otawebupdater.h>
#include <wifimanager.h>

RTC_DS3231 rtc;

WIFIMANAGER WifiManager;
OTAWEBUPDATER OtaWebUpdater;

AsyncWebServer *tcp_server;

AlpacaServer alpacaServer("Alpaca_ESP32");

SafetyMonitor safetymonitor = SafetyMonitor();
ObservingConditions observingconditions = ObservingConditions();

Meteo meteo("AlpacaESP32");

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

String mqttLogBuffer = "";

void logLine(String line) {
    Serial.println(line);
    webSerial.println(line);
    if (mqttClient)
        mqttClient->publish(MQTT_LOG_TOPIC, mqttLogBuffer + line);
    mqttLogBuffer = "";
}

void logLinePart(String line) {
    Serial.print(line);
    webSerial.print(line);
    mqttLogBuffer = mqttLogBuffer + line;
}

void logMessage(String msg, bool showtime = true) {
    if (showtime)
        logLinePart(logTime() + " ");
    logLine(msg);
}

void logMessagePart(String msg, bool showtime = false) {
    if (showtime)
        logLinePart(logTime() + " ");
    logLinePart(msg);
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

void IRAM_ATTR immediateMeteoUpdate() {
    immediateUpdate = true;
}

void setup() {
    // Setup serial
    Serial.begin(115200);
    while (!Serial) {
    }
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
    // TODO Fixed WiFI settings with reconnect
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
    webSerial.onMessage([](const std::string &msg) { Serial.println(msg.c_str()); });
    webSerial.setBuffer(100);
    webSerial.begin(tcp_server);
    // WiFi Manager
    WifiManager.setLogger(logLine, logLinePart, logTime); // Set message logger
    WifiManager.startBackgroundTask();                    // Run the background task to take care of our Wifi
    WifiManager.fallbackToSoftAp(true);                   // Run a SoftAP if no known AP can be reached
    WifiManager.attachWebServer(tcp_server);              // Attach our API to the HTTP Webserver
    WifiManager.attachUI();
    // OTA Manager
    // OtaWebUpdater.setBaseUrl(OTA_BASE_URL);    // Set the OTA Base URL for automatic updates
    OtaWebUpdater.setLogger(logLine, logLinePart, logTime); // Set message logger
    OtaWebUpdater.setFirmware(BUILD_DATE, VERSION);         // Set the current firmware version
    OtaWebUpdater.startBackgroundTask();                    // Run the background task to check for updates
    OtaWebUpdater.attachWebServer(tcp_server);              // Attach our API to the Webserver
    OtaWebUpdater.attachUI();                               // Attach the UI to the Webserver
    tcp_server->on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/www/favicon.ico", "image/x-icon");
    });
    tcp_server->on("/ascom.webp", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/www/ascom.webp", "image/webp");
    });
    tcp_server->begin();
    // ALPACA Tcp Server
    alpacaServer.setLogger(logLine, logLinePart, logTime);
    alpacaServer.beginTcp(tcp_server, ALPACA_TCP_PORT);
    // Observing Conditions
    alpacaServer.addDevice(&observingconditions);
    // Safety Monitor
    safetymonitor.setLogger(logLine, logLinePart, logTime);
    alpacaServer.addDevice(&safetymonitor);
    alpacaServer.loadSettings();
    // Meteo sensors
    meteo.setLogger(logLine, logLinePart, logTime);
    meteo.begin();
    attachInterrupt(digitalPinToInterrupt(RAIN_SENSOR_PIN), immediateMeteoUpdate, CHANGE);
}

void loop() {
    // weather sensors loop delay
    static unsigned long meteoMeasureDelay = METEO_MEASURE_DELAY;
    static unsigned long meteoLastTimeRan = 0;
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
    // MQTT Status
    if (millis() > lastMqttStatus + mqttStatusDelay) { // read every measureDelay without blocking Webserver
        logMqttStatus();
        lastMqttStatus = millis();
    }
    // Actual Load
    if (immediateUpdate || (millis() > meteoLastTimeRan + meteoMeasureDelay)) { // read every measureDelay without blocking Webserver
        meteo.update(meteoMeasureDelay);
        safetymonitor.update(meteo, meteoMeasureDelay);
        observingconditions.update(meteo, meteoMeasureDelay);
        meteoLastTimeRan = millis();
    }
    immediateUpdate = false;
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
