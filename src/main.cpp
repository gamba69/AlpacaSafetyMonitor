#include "main.h"
#include "console.h"
#include "hardware.h"
#include "log.h"
#include "secrets.h"
#include "version.h"
#include "weights.h"
#include <jled.h>

RTC_DS3231 rtc;

WebSerial webSerial;
PicoMQTT::Client *mqttClient = nullptr;
JLed led = JLed(LED_PIN).LowActive().Off();

WIFIMANAGER WifiManager;
OTAWEBUPDATER OtaWebUpdater;

AsyncWebServer *tcp_server;

AlpacaServer alpacaServer("Alpaca_ESP32", (VERSION + String(", build ") + BUILD_NUMBER).c_str(), BUILD_DATE);

SafetyMonitor safetymonitor = SafetyMonitor();
ObservingConditions observingconditions = ObservingConditions();
Meteo meteo;

EventGroupHandle_t xInterruptsGroup;

#define UICPAL_INTERRUPT (1UL << 0)
#define TSL2591_INTERRUPT (1UL << 1)
#define TSL2591_READY (1UL << 2)

volatile bool immediate = false;
volatile bool readiness = false;

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
    webServer->on("//wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->redirect("/wifi");
    });
}

void IRAM_ATTR uicpalInterruptHandler() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xEventGroupSetBitsFromISR(xInterruptsGroup, UICPAL_INTERRUPT, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

void IRAM_ATTR tslInterruptHandler() {
   static unsigned long last = 0;
   if (millis() - last > 1000) {
       last = millis();
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xEventGroupSetBitsFromISR(xInterruptsGroup, TSL2591_INTERRUPT, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
   }
}

void IRAM_ATTR tslDataReadyHandler() {
    xEventGroupSetBits(xInterruptsGroup, TSL2591_READY);
}

void workload(void *parameter) {
    // weather update last ran millis
    static unsigned long meteoLastRan = 0;
    static unsigned long safetyMonitorLastRan = 0;
    static unsigned long observingConditionsLastRan = 0;
    // mqtt status loop delay
    static int prevWifiStatus = WL_DISCONNECTED;
    static int mqttStatusDelay = MQTT_STATUS_DELAY;
    static int lastMqttStatus = 0;
    esp_task_wdt_add(NULL);
    while (true) {
        // led work always
        led.Update();
        // do not continue regular operation as long as a OTA is running
        // reason: background workload can cause upgrade issues that we want to avoid!
        if (OtaWebUpdater.otaIsRunning) {
            esp_task_wdt_reset();
            taskYIELD();
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
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
        if (immediate || readiness || (millis() > meteoLastRan + METEO_MEASURE_DELAY)) {
            meteo.update(immediate);
            meteoLastRan = millis();
        }
        // update observingconditions every refresh without blocking webserver
        if (ALPACA_OBSCON) {
            if (immediate || readiness || (millis() > observingConditionsLastRan + (1000 * observingconditions.getRefresh()))) {
                observingconditions.update(&meteo);
                observingConditionsLastRan = millis();
            }
        }
        // update safetymonitor every METEO_MEASURE_DELAY without blocking webserver
        if (ALPACA_SAFEMON) {
            if (immediate || readiness || (millis() > safetyMonitorLastRan + SAFETY_MONITOR_DELAY)) {
                safetymonitor.update(&meteo);
                safetyMonitorLastRan = millis();
            }
        }
        immediate = false;
        readiness = false;
        esp_task_wdt_reset();
        taskYIELD();
        EventBits_t await = UICPAL_INTERRUPT | TSL2591_INTERRUPT | TSL2591_READY;
        xEventGroupClearBits(xInterruptsGroup, await);
        EventBits_t xBits = xEventGroupWaitBits(
            xInterruptsGroup,
            await,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(50));
        if ((xBits & UICPAL_INTERRUPT) != 0) {
            logTechMessage("[TECH][UICPAL] Rain state changed, immediate update");
            immediate = true;
        }
        if ((xBits & TSL2591_INTERRUPT) != 0) {
            logTechMessage("[TECH][TSL2591] Thresholds exceeded, immediate update");
            immediate = true;
        }
        if ((xBits & TSL2591_READY) != 0) {
            logTechMessage("[TECH][TSL2591] Data ready, update");
            readiness = true;
        }
    }
    esp_task_wdt_delete(NULL);
    vTaskDelete(NULL);
}

void setup() {
    // Setup serial
    Serial.begin(115200);
    Serial.println("");
    // Logging preferences
    initLogPrefs();
    // Hardware preferences
    initHwPrefs();
    // Temperature and humidity calculation preferences
    initThWeightsPrefs();
    // System Timezone
    setenv("TZ", RTC_TIMEZONE, 1);
    tzset();
    // RTC
    if (HARDWARE_DS3231) {
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
    }
    // TODO Fixed WiFi settings with reconnect
    // setup_wifi();
    // NTP
    NTP.onNTPSyncEvent([](NTPEvent_t event) {
        switch (event.event) {
        case timeSyncd: {
            if (HARDWARE_DS3231) {
                struct timeval now;
                gettimeofday(&now, NULL);
                time_t t = now.tv_sec;
                rtc.adjust(DateTime(t));
                logMessage("[TIME][RTC] Synced");
            }
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
    initConsoleCommands();
    webSerial.onMessage(processConsoleCommand);
    webSerial.setBuffer(100);
    webSerial.begin(tcp_server);
    // WiFi Manager
    WifiManager.setLogger(LogSource::Wifi, logLine, logLinePart, logTime); // Set message logger
    WifiManager.startBackgroundTask();                                     // Run the background task to take care of our Wifi
    WifiManager.fallbackToSoftAp(true);                                    // Run a SoftAP if no known AP can be reached
    WifiManager.attachWebServer(tcp_server);                               // Attach our API to the HTTP Webserver
    WifiManager.attachUI();
    // OTA Manager
    // OtaWebUpdater.setBaseUrl(OTA_BASE_URL);    // Set the OTA Base URL for automatic updates
    OtaWebUpdater.setLogger(LogSource::Ota, logLine, logLinePart, logTime);                     // Set message logger
    OtaWebUpdater.setFirmware(BUILD_DATE, String(VERSION) + ", build " + String(BUILD_NUMBER)); // Set the current firmware version
    OtaWebUpdater.startBackgroundTask();                                                        // Run the background task to check for updates
    OtaWebUpdater.attachWebServer(tcp_server);                                                  // Attach our API to the Webserver
    OtaWebUpdater.attachUI();                                                                   // Attach the UI to the Webserver
    setupWebImages(tcp_server);
    setupWebMainPage(tcp_server);
    setupWebRedirects(tcp_server);
    tcp_server->begin();
    // ALPACA Tcp Server
    alpacaServer.setLogger(LogSource::Alpaca, logLine, logLinePart, logTime);
    alpacaServer.beginTcp(tcp_server, ALPACA_TCP_PORT);
    // Observing Conditions
    if (ALPACA_OBSCON) {
        observingconditions.setImmediateUpdate(uicpalInterruptHandler);
        observingconditions.setLogger(LogSource::ObsCon, logLine, logLinePart, logTime);
        alpacaServer.addDevice(&observingconditions);
    }
    // Safety Monitor
    if (ALPACA_SAFEMON) {
        safetymonitor.setLogger(LogSource::SafeMon, logLine, logLinePart, logTime);
        alpacaServer.addDevice(&safetymonitor);
    }
    alpacaServer.loadSettings();
    // Meteo sensors
    meteo.getTsl2591()->setDataReadyCallback(tslDataReadyHandler);
    meteo.setLogger(LogSource::Meteo, logLine, logLinePart, logTime);
    meteo.begin();
    if (HARDWARE_UICPAL) {
        attachInterrupt(digitalPinToInterrupt(RAIN_SENSOR_PIN), uicpalInterruptHandler, CHANGE);
    }
    if (HARDWARE_TSL2591) {
        attachInterrupt(digitalPinToInterrupt(TSL_SENSOR_PIN), tslInterruptHandler, FALLING);
    }
    // Watchdog
    esp_task_wdt_deinit();
    esp_err_t error = esp_task_wdt_init(WATCHDOG_COUNTDOWN, true);
    logMessage("[WATCHDOG] Initialization: " + String(esp_err_to_name(error)));
    // Tasks
    xInterruptsGroup = xEventGroupCreate();
    xTaskCreate(
        workload,   // Function to implement the task
        "workload", // Name of the task
        8192,       // Stack size in bytes
        NULL,       // Task input parameter
        1,          // Priority of the task
        NULL        // Task handle
    );
    // Terminate
    vTaskDelete(NULL);
}

void loop() {
    // Indeed
}

void setup_wifi() {
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
