#include "WiFiHandler.h"
#include "NotificationManager.h"

WiFiHandler::WiFiHandler(std::string ssid, std::string password) : ssid(ssid), password(password) {
    // Start always as STA mode
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    // Wait for connection with timeout (30 seconds)
    unsigned long startTime = millis();
    const unsigned long timeout = 30000; // 30 seconds

    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
        vTaskDelay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
        connected = true;
        DEBUG_PRINTLN("[WiFi] Connected to STA: " << ssid << " (" << WiFi.localIP().toString().c_str() << ")");
        NotificationManager::getInstance().postNotification(
            "Connected",
            ssid,
            5000
        );
    } else {
        connected = false;
        DEBUG_PRINTLN("[WiFi] Failed to connect to STA, timeout reached");
    }

    // Initialize DNS server (will be started only in AP mode)
    dns = new DNSServer();

    // Start mDNS
    if(!MDNS.begin(DNS_NAME)) {
        DEBUG_PRINTLN("[mDNS] Failed to start");
    } else {
        DEBUG_PRINTLN("[mDNS] Started: " << DNS_NAME);
    }

    // Initialize web server
    server = new AsyncWebServer(80);

    // Setup OTA
    #if DISABLE_OTA == true
        DEBUG_PRINTLN("[OTA] DISABLED");
    #elif defined(OTA_SSID) && defined(OTA_PASS)
        ElegantOTA.begin(server, OTA_SSID, OTA_PASS);
    #else
        ElegantOTA.begin(server);
    #endif

    // Setup web server callbacks
    IPAddress localIP = connected ? WiFi.localIP() : IPAddress(192, 168, 4, 1); // Default AP IP if not connected
    webServerCallbacks(*server, localIP);

    // Start web server
    server->begin();
    DEBUG_PRINTLN("[WebServer] Started on port 80");
}

WiFiHandler::~WiFiHandler() {
    if (server) {
        server->end();
        delete server;
        server = nullptr;
    }
    if (dns) {
        dns->stop();
        delete dns;
        dns = nullptr;
    }
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

void WiFiHandler::RunTask(void){
    xTaskCreatePinnedToCore(TaskEntry,      // Task entry point
                            "WiFiTask",     // Task name
                            4096,           // Stack depth
                            this,           // Task parameters - pointer to this instance
                            2,              // Task priority (HIGH - UI responsiveness)
                            NULL,           // Task handle
                            1);             // Core ID (Core 0 - Fast Core)
}

void WiFiHandler::TaskEntry(void* param) {
    WiFiHandler* instance = static_cast<WiFiHandler*>(param);

    while (true) {
        instance->Update();
        vTaskDelay(1000 / 10);
    }
}

// Set callbacks from main.cpp
void WiFiHandler::setOnStart(std::function<void()> callback) {
    ElegantOTA.onStart(callback);
}

void WiFiHandler::setOnProgress(std::function<void(size_t, size_t)> callback) {
    ElegantOTA.onProgress(callback);
}

void WiFiHandler::setOnEnd(std::function<void(bool)> callback) {
    ElegantOTA.onEnd(callback);
}


void WiFiHandler::Update() {
    // Process DNS server requests (needed for captive portal in AP mode)
    if (dns && (mode == WIFI_AP || mode == WIFI_AP_STA)) {
        dns->processNextRequest();
    }

    // Update connection status in STA mode
    if (mode == WIFI_STA) {
        bool wasConnected = connected;
        connected = (WiFi.status() == WL_CONNECTED);

        if (wasConnected != connected) {
            if (connected) {
                DEBUG_PRINTLN("[WiFi] Reconnected to STA");
                DEBUG_PRINTLN("[WiFi] IP Address: " << WiFi.localIP().toString());

                // Post notification: "Connected, Wifi SSID" for 5 seconds
                NotificationManager::getInstance().postNotification(
                    "Connected",
                    ssid,
                    5000
                );
            } else {
                DEBUG_PRINTLN("[WiFi] Disconnected from STA");

                // Post notification: "Disconnected" for 5 seconds
                NotificationManager::getInstance().postNotification(
                    "Disconnected",
                    "Connection lost",
                    5000
                );
            }
        }
    }
}


void WiFiHandler::SwitchMode(WiFiMode_t newMode) {
    DEBUG_PRINTLN("[WiFi] Switching mode from " << (mode == WIFI_STA ? "STA" : "AP") <<
                  " to " << (newMode == WIFI_STA ? "STA" : "AP"));

    // Stop DNS server if running
    if (dns && (mode == WIFI_AP || mode == WIFI_AP_STA)) {
        dns->stop();
        DEBUG_PRINTLN("[DNS] Stopped");
    }

    // Disconnect current connection
    WiFi.disconnect();
    vTaskDelay(100);

    // Set new mode
    this->mode = newMode;
    WiFi.mode(newMode);

    if (newMode == WIFI_STA) {
        // Connect to STA
        WiFi.begin(ssid.c_str(), password.c_str());

        // Wait for connection with timeout (30 seconds)
        unsigned long startTime = millis();
        const unsigned long timeout = 30000;

        while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
            vTaskDelay(500);
        }

        if (WiFi.status() == WL_CONNECTED) {
            connected = true;
            DEBUG_PRINTLN("[WiFi] Connected to STA: " << ssid);
            DEBUG_PRINTLN("[WiFi] IP Address: " << WiFi.localIP().toString());

            // Update web server callbacks with new IP
            webServerCallbacks(*server, WiFi.localIP());

            // Post notification: "Connected, Wifi SSID" for 5 seconds
            NotificationManager::getInstance().postNotification(
                "Connected",
                ssid,
                5000
            );
        } else {
            connected = false;
            DEBUG_PRINTLN("[WiFi] Failed to connect to STA");
        }
    } else if (newMode == WIFI_AP || newMode == WIFI_AP_STA) {
        // Start AP mode
        WiFi.softAP(AP_SSID, AP_PASS);
        vTaskDelay(500); // Give AP time to start

        IPAddress apIP = WiFi.softAPIP();
        DEBUG_PRINTLN("[WiFi] AP Started: " << AP_SSID);
        DEBUG_PRINTLN("[WiFi] AP IP Address: " << apIP.toString());

        // Start DNS server for captive portal
        if (dns) {
            dns->start(53, "*", apIP);
            DEBUG_PRINTLN("[DNS] Started for captive portal");
        }

        // Update web server callbacks with AP IP
        webServerCallbacks(*server, apIP);

        // In AP mode, we're always "connected" (AP is up)
        connected = true;

        // Post notification: "Configuration, MDNS\nAddressIP" (no timeout)
        std::string apContent = std::string(DNS_NAME) + "\n" + apIP.toString().c_str();
        NotificationManager::getInstance().postNotification(
            "Configuration",
            apContent,
            0  // 0 = no timeout (persistent)
        );
    }
}