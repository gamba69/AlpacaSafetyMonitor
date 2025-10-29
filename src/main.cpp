#include "main.h"
#include "otawebupdater.h"
#include "secrets.h"
#include "wifimanager.h"

RTC_DS3231 rtc;

WIFIMANAGER WifiManager;
OTAWEBUPDATER OtaWebUpdater;

AsyncWebServer *tcp_server;
AsyncUDP udp_server;

AlpacaServer alpacaServer("Alpaca_ESP32");

SafetyMonitor safetymonitor = SafetyMonitor();
ObservingConditions observingconditions = ObservingConditions();

void setup() {
    // setup serial
    Serial.begin(115200);
    while (!Serial) {
        // wait for serial port to connect.
    }
    delay(3000);
    Serial.println("[SAFEMON] Serial is ready");

    // setup_wifi();

    tcp_server = new AsyncWebServer(ALPACA_TCP_PORT);

    WifiManager.startBackgroundTask();       // Run the background task to take care of our Wifi
    WifiManager.fallbackToSoftAp(true);      // Run a SoftAP if no known AP can be reached
    WifiManager.attachWebServer(tcp_server); // Attach our API to the HTTP Webserver
    WifiManager.attachUI();

    // OtaWebUpdater.setBaseUrl(OTA_BASE_URL);        // Set the OTA Base URL for automatic updates
    OtaWebUpdater.setFirmware(__DATE__, "1.0.0"); // Set the current firmware version
    OtaWebUpdater.startBackgroundTask();          // Run the background task to check for updates
    OtaWebUpdater.attachWebServer(tcp_server);    // Attach our API to the Webserver
    OtaWebUpdater.attachUI();                     // Attach the UI to the Webserver

    tcp_server->begin();

    // setup ASCOM Alpaca server
    udp_server.listen(ALPACA_UDP_PORT);
    alpacaServer.begin(&udp_server, ALPACA_UDP_PORT, tcp_server, ALPACA_TCP_PORT);
    // alpacaServer.debug;   // uncoment to get Server messages in Serial monitor
    alpacaServer.addDevice(&safetymonitor);
    alpacaServer.addDevice(&observingconditions);
    alpacaServer.loadSettings();

    meteo.setup_i2c();

    // if (!rtc.begin()) {
    //     Serial.println("Couldn't find RTC");
    // }

    // if (rtc.lostPower()) {
    //     Serial.println("RTC lost power, let's set the time!");
    //     rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // }

    // Serial.print("[SAFEMON] RTC ");
    // char buffer[30];
    // strcpy(buffer, "YYYY-MM-DD hh:mm:ss"); // Example format: 2025-10-27 19:57:00
    // Serial.println(rtc.now().toString(buffer));
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
        meteo.update_i2c(meteoMeasureDelay);
        safetymonitor.update(meteo, meteoMeasureDelay);
        observingconditions.update(meteo, meteoMeasureDelay);
        meteoLastTimeRan = millis();
    }
    delay(50);
}

void setup_wifi() {
    // pinMode(PIN_WIFI_LED, OUTPUT);
    Serial.println(F("[SAFEMON] Setup WiFi"));
    WiFi.begin(WIFISSID, WIFIPASS);
    int pause = 10000;
    unsigned long begin = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - begin < pause) {
        delay(500);
        Serial.print(".");
    }
    Serial.print("[SAFEMON] WiFi connected: ");
    Serial.println(WiFi.localIP());
}
