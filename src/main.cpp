#include "main.h"
#include "otawebupdater.h"
#include "secrets.h"
#include "wifimanager.h"
#include <MycilaWebSerial.h>

RTC_DS3231 rtc;

WIFIMANAGER WifiManager;
OTAWEBUPDATER OtaWebUpdater;

AsyncWebServer *tcp_server;
AsyncUDP udp_server;

AlpacaServer alpacaServer("Alpaca_ESP32");

SafetyMonitor safetymonitor = SafetyMonitor();
ObservingConditions observingconditions = ObservingConditions();

Meteo meteo("AlpacaESP32");

WebSerial webSerial;

String logTime() {
    time_t now;
    struct tm timeinfo;
    time(&now);
    setenv("TZ", "Europe/Kiev", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    // gmtime_r(&now, &timeinfo);
    char strftime_buf[64]; // Ensure buffer is large enough
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(strftime_buf);
}

void logLine(String line) {
    Serial.println(line);
    webSerial.println(line);
}

void logLinePart(String line) {
    Serial.print(line);
    webSerial.print(line);
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

void setup() {

    // Setup serial
    Serial.begin(115200);
    while (!Serial) {
    }
    delay(4000);
    Stream *stream;
    // RTC
    if (!rtc.begin())
        logMessage("[MAIN] Couldn't find RTC", false);

    // Init unpowered RTC
    if (rtc.lostPower()) {
        logMessage("[MAIN] RTC lost power, let's set the time!", false);
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    // Read time from RTC
    DateTime rtcNow = rtc.now();
    struct timeval tv;
    tv.tv_sec = rtcNow.unixtime();
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);

    // TODO Fixed WiFI settings with reconnect
    // setup_wifi();

    // TCP server
    tcp_server = new AsyncWebServer(ALPACA_TCP_PORT);

    webSerial.onMessage([](const std::string &msg) { Serial.println(msg.c_str()); });
    webSerial.setBuffer(100);
    webSerial.begin(tcp_server);

    // WiFi Manager
    WifiManager.setLogger(&Serial, logTime); // Set message logger
    WifiManager.startBackgroundTask();       // Run the background task to take care of our Wifi
    WifiManager.fallbackToSoftAp(true);      // Run a SoftAP if no known AP can be reached
    WifiManager.attachWebServer(tcp_server); // Attach our API to the HTTP Webserver
    WifiManager.attachUI();

    // OTA Manager
    // TODO Versions!
    // OtaWebUpdater.setBaseUrl(OTA_BASE_URL);    // Set the OTA Base URL for automatic updates
    OtaWebUpdater.setLogger(&Serial, logTime);    // Set message logger
    OtaWebUpdater.setFirmware(__DATE__, "1.0.0"); // Set the current firmware version
    OtaWebUpdater.startBackgroundTask();          // Run the background task to check for updates
    OtaWebUpdater.attachWebServer(tcp_server);    // Attach our API to the Webserver
    OtaWebUpdater.attachUI();                     // Attach the UI to the Webserver

    tcp_server->begin();

    // UDP Server
    udp_server.listen(ALPACA_UDP_PORT);

    // ALPACA Server
    alpacaServer.setLogger(&Serial, logTime);
    alpacaServer.begin(&udp_server, ALPACA_UDP_PORT, tcp_server, ALPACA_TCP_PORT);
    // Observing Conditions
    alpacaServer.addDevice(&observingconditions);
    // Safety Monitor
    safetymonitor.setLogger(logLine, logLinePart, logTime);
    alpacaServer.addDevice(&safetymonitor);
    alpacaServer.loadSettings();

    // Meteo sensors
    meteo.setLogger(logLine, logLinePart, logTime);
    meteo.begin();
}

void loop() {
    // Do not continue regular operation as long as a OTA is running
    // Reason: Background workload can cause upgrade issues that we want to avoid!
    if (OtaWebUpdater.otaIsRunning) {
        yield();
        delay(50);
        return;
    };
    // Actual Load
    if (millis() > meteoLastTimeRan + meteoMeasureDelay) { // read every measureDelay without blocking Webserver
        meteo.update(meteoMeasureDelay);
        safetymonitor.update(meteo, meteoMeasureDelay);
        observingconditions.update(meteo, meteoMeasureDelay);
        meteoLastTimeRan = millis();

        // Serial.print("[SAFEMON] RTC ");
        // char buffer[30];
        // strcpy(buffer, "YYYY-MM-DD hh:mm:ss"); // Example format: 2025-10-27 19:57:00
        // Serial.println(rtc.now().toString(buffer));
    }
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
